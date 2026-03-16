#include <cassert>
#include <iostream>
#include <string>

#include "kinbot/world_state/world_state_store.h"

namespace {

using kinbot::common::v1::ANOMALY_ACTIVE;
using kinbot::common::v1::CRITICAL;
using kinbot::common::v1::PERSONAL_SENSITIVE;
using kinbot::event_bus::v1::WorldEvent;
using kinbot::event_bus::v1::WorldEventType;
using kinbot::platform::world_state::WorldStateStore;

WorldEvent MakeEvent(WorldEventType type,
                     std::initializer_list<std::pair<std::string, std::string>> payload,
                     int64_t timestamp_ms) {
  WorldEvent event;
  event.set_event_id("test_event_" + std::to_string(timestamp_ms));
  event.set_event_type(type);
  event.set_timestamp_ms(timestamp_ms);
  event.set_source_module("world_state_sil_test");
  event.set_privacy_level(PERSONAL_SENSITIVE);

  for (const auto& [key, value] : payload) {
    (*event.mutable_payload())[key] = value;
  }

  return event;
}

void AssertPersonIdentificationFlow(WorldStateStore* store) {
  const auto result = store->ApplyEvent(MakeEvent(
      kinbot::event_bus::v1::PERSON_IDENTIFIED,
      {{"person_id", "elder_1"},
       {"display_name", "Grandma"},
       {"age_group", "elder"},
       {"default_location", "bedroom"}},
      1000));

  assert(result.applied);
  const auto& snapshot = store->snapshot();
  assert(snapshot.snapshot_version() == 1);
  assert(snapshot.snapshot_state().active_persons_size() == 1);
  assert(snapshot.snapshot_state().active_persons(0).person_id() == "elder_1");
  assert(snapshot.snapshot_state().active_persons(0).display_name() == "Grandma");
}

void AssertFallEscalationFlow(WorldStateStore* store) {
  const auto result = store->ApplyEvent(
      MakeEvent(kinbot::event_bus::v1::FALL_SUSPECTED,
                {{"person_id", "elder_1"},
                 {"risk_event_id", "fall_001"},
                 {"severity", "CRITICAL"},
                 {"status", "confirmed"}},
                2000));

  assert(result.applied);
  const auto& snapshot = store->snapshot();
  assert(snapshot.snapshot_version() == 2);
  assert(snapshot.snapshot_state().health_alerts_size() == 1);
  assert(snapshot.snapshot_state().health_alerts(0).risk_event_id() == "fall_001");
  assert(snapshot.snapshot_state().health_alerts(0).severity() == CRITICAL);
  assert(snapshot.snapshot_state().home_mode() == ANOMALY_ACTIVE);
}

void AssertManualServiceTransitions(WorldStateStore* store) {
  assert(store->ApplyEvent(
                   MakeEvent(kinbot::event_bus::v1::MANUAL_SERVICE_REQUESTED, {},
                             3000))
             .applied);
  assert(store->snapshot().snapshot_state().manual_service_state() == "requested");

  assert(store->ApplyEvent(
                   MakeEvent(kinbot::event_bus::v1::MANUAL_SERVICE_CONNECTED, {},
                             3100))
             .applied);
  assert(store->snapshot().snapshot_state().manual_service_state() == "connected");

  assert(store->ApplyEvent(
                   MakeEvent(kinbot::event_bus::v1::MANUAL_SERVICE_TIMEOUT, {}, 3200))
             .applied);
  assert(store->snapshot().snapshot_state().manual_service_state() == "timeout");
}

void AssertMedicationTaskFlow(WorldStateStore* store) {
  const auto due_result = store->ApplyEvent(
      MakeEvent(kinbot::event_bus::v1::MEDICATION_DUE,
                {{"task_id", "med_task_001"},
                 {"person_id", "elder_1"},
                 {"task_type", "remind"},
                 {"priority", "urgent"}},
                4000));
  assert(due_result.applied);
  assert(store->snapshot().snapshot_state().medication_urgency());
  assert(store->snapshot().snapshot_state().active_tasks_size() == 1);
  assert(store->snapshot().snapshot_state().active_tasks(0).status() == "pending");

  const auto complete_result = store->ApplyEvent(
      MakeEvent(kinbot::event_bus::v1::TASK_STATE_CHANGED,
                {{"task_id", "med_task_001"},
                 {"status", "completed"},
                 {"approval_status", "approved"}},
                5000));
  assert(complete_result.applied);

  const auto& snapshot = store->snapshot();
  assert(snapshot.snapshot_state().active_tasks(0).status() == "completed");
  assert(snapshot.snapshot_state().active_tasks(0).approval_status() == "approved");
  assert(!snapshot.snapshot_state().medication_urgency());
}

}  // namespace

int main() {
  WorldStateStore store;

  AssertPersonIdentificationFlow(&store);
  AssertFallEscalationFlow(&store);
  AssertManualServiceTransitions(&store);
  AssertMedicationTaskFlow(&store);

  std::cout << "world_state SIL flow test passed\n";
  return 0;
}
