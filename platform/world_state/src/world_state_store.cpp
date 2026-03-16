#include "kinbot/world_state/world_state_store.h"

#include <google/protobuf/repeated_ptr_field.h>

#include <unordered_map>
#include <utility>

namespace kinbot::platform::world_state {
namespace {
using WorldEvent = kinbot::event_bus::v1::WorldEvent;

template <typename T, typename Predicate>
T* FindBy(google::protobuf::RepeatedPtrField<T>* field, Predicate predicate) {
  for (int index = 0; index < field->size(); ++index) {
    auto* item = field->Mutable(index);
    if (predicate(*item)) {
      return item;
    }
  }
  return nullptr;
}

}  // namespace

WorldStateStore::WorldStateStore() {
  auto* state = snapshot_.mutable_snapshot_state();
  state->set_home_mode(kinbot::common::v1::DAYTIME);
  state->set_manual_service_state("none");
  state->mutable_network_state()->set_status("unknown");
}

ApplyResult WorldStateStore::ApplyEvent(const WorldEvent& event) {
  ApplyResult result;

  switch (event.event_type()) {
    case kinbot::event_bus::v1::PERSON_IDENTIFIED:
      result = ApplyPersonIdentified(event);
      break;
    case kinbot::event_bus::v1::FALL_SUSPECTED:
      result = ApplyFallSuspected(event);
      break;
    case kinbot::event_bus::v1::MEDICATION_DUE:
      result = ApplyMedicationDue(event);
      break;
    case kinbot::event_bus::v1::TASK_STATE_CHANGED:
      result = ApplyTaskStateChanged(event);
      break;
    case kinbot::event_bus::v1::MANUAL_SERVICE_REQUESTED:
      result = ApplyManualServiceState(event, "requested");
      break;
    case kinbot::event_bus::v1::MANUAL_SERVICE_CONNECTED:
      result = ApplyManualServiceState(event, "connected");
      break;
    case kinbot::event_bus::v1::MANUAL_SERVICE_TIMEOUT:
      result = ApplyManualServiceState(event, "timeout");
      break;
    default:
      result.message =
          "event type is not implemented by the bootstrap world_state reducer";
      return result;
  }

  if (result.applied) {
    FinalizeMutation(event);
  }

  return result;
}

ApplyResult WorldStateStore::ApplyPersonIdentified(const WorldEvent& event) {
  const std::string* person_id = FindPayload(event, "person_id");
  if (person_id == nullptr || person_id->empty()) {
    return {false, "PERSON_IDENTIFIED requires payload.person_id"};
  }

  auto* person = UpsertActivePerson(*person_id, event.timestamp_ms(), event);
  person->set_person_id(*person_id);
  person->set_identity_status("identified");
  person->set_display_name(PayloadOr(event, "display_name", *person_id));
  person->set_age_group(PayloadOr(event, "age_group", "elder"));
  person->set_mobility_level(PayloadOr(event, "mobility_level", "unknown"));
  person->set_default_location(PayloadOr(event, "default_location", ""));
  person->set_care_priority(PayloadOr(event, "care_priority", "normal"));

  return {true, "active person upserted"};
}

ApplyResult WorldStateStore::ApplyFallSuspected(const WorldEvent& event) {
  const std::string* person_id = FindPayload(event, "person_id");
  if (person_id == nullptr || person_id->empty()) {
    return {false, "FALL_SUSPECTED requires payload.person_id"};
  }

  auto* alert =
      UpsertHealthAlert(PayloadOr(event, "risk_event_id", event.event_id()));
  alert->set_risk_event_id(PayloadOr(event, "risk_event_id", event.event_id()));
  alert->set_risk_type(PayloadOr(event, "risk_type", "fall"));
  alert->set_subject_person_id(*person_id);
  alert->set_severity(ParseRiskLevel(PayloadOr(event, "severity", "HIGH")));
  alert->set_status(PayloadOr(event, "status", "candidate"));

  snapshot_.mutable_snapshot_state()->set_home_mode(
      kinbot::common::v1::ANOMALY_ACTIVE);
  return {true, "health alert upserted and home mode escalated"};
}

ApplyResult WorldStateStore::ApplyMedicationDue(const WorldEvent& event) {
  const std::string* task_id = FindPayload(event, "task_id");
  if (task_id == nullptr || task_id->empty()) {
    return {false, "MEDICATION_DUE requires payload.task_id"};
  }

  auto* task = UpsertTask(*task_id);
  task->set_task_id(*task_id);
  task->set_task_type(PayloadOr(event, "task_type", "remind"));
  task->set_owner_person_id(PayloadOr(event, "person_id", ""));
  task->set_trigger_source(PayloadOr(event, "trigger_source", "scheduled"));
  task->set_priority(PayloadOr(event, "priority", "high"));
  task->set_status(PayloadOr(event, "status", "pending"));
  task->set_approval_status(PayloadOr(event, "approval_status", "pending"));

  snapshot_.mutable_snapshot_state()->set_medication_urgency(true);
  return {true, "medication task upserted and urgency raised"};
}

ApplyResult WorldStateStore::ApplyTaskStateChanged(const WorldEvent& event) {
  const std::string* task_id = FindPayload(event, "task_id");
  if (task_id == nullptr || task_id->empty()) {
    return {false, "TASK_STATE_CHANGED requires payload.task_id"};
  }

  const std::string* status = FindPayload(event, "status");
  if (status == nullptr || status->empty()) {
    return {false, "TASK_STATE_CHANGED requires payload.status"};
  }

  auto* task = UpsertTask(*task_id);
  task->set_task_id(*task_id);

  if (const std::string* task_type = FindPayload(event, "task_type");
      task_type != nullptr && !task_type->empty()) {
    task->set_task_type(*task_type);
  }
  if (const std::string* owner_person_id = FindPayload(event, "person_id");
      owner_person_id != nullptr) {
    task->set_owner_person_id(*owner_person_id);
  }
  if (const std::string* approval = FindPayload(event, "approval_status");
      approval != nullptr && !approval->empty()) {
    task->set_approval_status(*approval);
  }

  task->set_status(*status);

  if (task->task_type() == "remind" && *status == "completed") {
    snapshot_.mutable_snapshot_state()->set_medication_urgency(false);
  }

  return {true, "task state updated"};
}

ApplyResult WorldStateStore::ApplyManualServiceState(
    const WorldEvent& event, std::string_view next_state) {
  snapshot_.mutable_snapshot_state()->set_manual_service_state(
      std::string(next_state));
  return {true, "manual service state updated"};
}

void WorldStateStore::FinalizeMutation(const WorldEvent& event) {
  snapshot_.set_snapshot_version(snapshot_.snapshot_version() + 1);
  snapshot_.mutable_snapshot_state()->set_timestamp_ms(event.timestamp_ms());
}

kinbot::world_state::v1::Person* WorldStateStore::UpsertActivePerson(
    const std::string& person_id, int64_t timestamp_ms, const WorldEvent& event) {
  auto* persons = snapshot_.mutable_snapshot_state()->mutable_active_persons();
  auto* person =
      FindBy(persons, [&person_id](const auto& item) { return item.person_id() == person_id; });
  if (person == nullptr) {
    person = persons->Add();
    person->mutable_meta()->set_created_at_ms(timestamp_ms);
  }

  auto* meta = person->mutable_meta();
  meta->set_id(person_id);
  meta->set_type("person");
  meta->set_version(meta->version() + 1);
  meta->set_updated_at_ms(timestamp_ms);
  meta->set_source(event.source_module());
  meta->set_confidence(1.0F);
  meta->set_privacy_level(event.privacy_level());

  return person;
}

kinbot::world_state::v1::RiskEvent* WorldStateStore::UpsertHealthAlert(
    const std::string& risk_event_id) {
  auto* alerts = snapshot_.mutable_snapshot_state()->mutable_health_alerts();
  auto* alert =
      FindBy(alerts, [&risk_event_id](const auto& item) { return item.risk_event_id() == risk_event_id; });
  if (alert == nullptr) {
    alert = alerts->Add();
  }
  return alert;
}

kinbot::world_state::v1::Task* WorldStateStore::UpsertTask(
    const std::string& task_id) {
  auto* tasks = snapshot_.mutable_snapshot_state()->mutable_active_tasks();
  auto* task =
      FindBy(tasks, [&task_id](const auto& item) { return item.task_id() == task_id; });
  if (task == nullptr) {
    task = tasks->Add();
  }
  return task;
}

const std::string* WorldStateStore::FindPayload(const WorldEvent& event,
                                                std::string_view key) {
  const auto& payload = event.payload();
  auto iterator = payload.find(std::string(key));
  if (iterator == payload.end()) {
    return nullptr;
  }
  return &iterator->second;
}

std::string WorldStateStore::PayloadOr(const WorldEvent& event,
                                       std::string_view key,
                                       std::string_view fallback) {
  const std::string* value = FindPayload(event, key);
  if (value == nullptr) {
    return std::string(fallback);
  }
  return *value;
}

kinbot::common::v1::RiskLevel WorldStateStore::ParseRiskLevel(
    std::string_view value) {
  static const std::unordered_map<std::string, kinbot::common::v1::RiskLevel>
      kRiskLevels = {
          {"LOW", kinbot::common::v1::LOW},
          {"MEDIUM", kinbot::common::v1::MEDIUM},
          {"HIGH", kinbot::common::v1::HIGH},
          {"CRITICAL", kinbot::common::v1::CRITICAL},
  };

  auto iterator = kRiskLevels.find(std::string(value));
  if (iterator == kRiskLevels.end()) {
    return kinbot::common::v1::HIGH;
  }
  return iterator->second;
}

bool ParseWorldEventType(std::string_view value,
                         kinbot::event_bus::v1::WorldEventType* out) {
  static const std::unordered_map<std::string, kinbot::event_bus::v1::WorldEventType>
      kEventTypes = {
          {"PERSON_IDENTIFIED", kinbot::event_bus::v1::PERSON_IDENTIFIED},
          {"FALL_SUSPECTED", kinbot::event_bus::v1::FALL_SUSPECTED},
          {"MEDICATION_DUE", kinbot::event_bus::v1::MEDICATION_DUE},
          {"TASK_STATE_CHANGED", kinbot::event_bus::v1::TASK_STATE_CHANGED},
          {"MANUAL_SERVICE_REQUESTED",
           kinbot::event_bus::v1::MANUAL_SERVICE_REQUESTED},
          {"MANUAL_SERVICE_CONNECTED",
           kinbot::event_bus::v1::MANUAL_SERVICE_CONNECTED},
          {"MANUAL_SERVICE_TIMEOUT",
           kinbot::event_bus::v1::MANUAL_SERVICE_TIMEOUT},
  };

  auto iterator = kEventTypes.find(std::string(value));
  if (iterator == kEventTypes.end()) {
    return false;
  }

  *out = iterator->second;
  return true;
}

std::string WorldEventTypeToString(kinbot::event_bus::v1::WorldEventType type) {
  switch (type) {
    case kinbot::event_bus::v1::PERSON_IDENTIFIED:
      return "PERSON_IDENTIFIED";
    case kinbot::event_bus::v1::FALL_SUSPECTED:
      return "FALL_SUSPECTED";
    case kinbot::event_bus::v1::MEDICATION_DUE:
      return "MEDICATION_DUE";
    case kinbot::event_bus::v1::TASK_STATE_CHANGED:
      return "TASK_STATE_CHANGED";
    case kinbot::event_bus::v1::MANUAL_SERVICE_REQUESTED:
      return "MANUAL_SERVICE_REQUESTED";
    case kinbot::event_bus::v1::MANUAL_SERVICE_CONNECTED:
      return "MANUAL_SERVICE_CONNECTED";
    case kinbot::event_bus::v1::MANUAL_SERVICE_TIMEOUT:
      return "MANUAL_SERVICE_TIMEOUT";
    default:
      return "WORLD_EVENT_TYPE_UNSPECIFIED";
  }
}

bool ParsePrivacyLevel(std::string_view value,
                       kinbot::common::v1::PrivacyLevel* out) {
  static const std::unordered_map<std::string, kinbot::common::v1::PrivacyLevel>
      kPrivacyLevels = {
          {"PUBLIC_RUNTIME", kinbot::common::v1::PUBLIC_RUNTIME},
          {"PERSONAL_SENSITIVE", kinbot::common::v1::PERSONAL_SENSITIVE},
          {"BIOMETRIC_SENSITIVE", kinbot::common::v1::BIOMETRIC_SENSITIVE},
          {"MEDICAL_SENSITIVE", kinbot::common::v1::MEDICAL_SENSITIVE},
  };

  auto iterator = kPrivacyLevels.find(std::string(value));
  if (iterator == kPrivacyLevels.end()) {
    return false;
  }

  *out = iterator->second;
  return true;
}

std::string PrivacyLevelToString(kinbot::common::v1::PrivacyLevel level) {
  switch (level) {
    case kinbot::common::v1::PUBLIC_RUNTIME:
      return "PUBLIC_RUNTIME";
    case kinbot::common::v1::PERSONAL_SENSITIVE:
      return "PERSONAL_SENSITIVE";
    case kinbot::common::v1::BIOMETRIC_SENSITIVE:
      return "BIOMETRIC_SENSITIVE";
    case kinbot::common::v1::MEDICAL_SENSITIVE:
      return "MEDICAL_SENSITIVE";
    default:
      return "PRIVACY_LEVEL_UNSPECIFIED";
  }
}

}  // namespace kinbot::platform::world_state
