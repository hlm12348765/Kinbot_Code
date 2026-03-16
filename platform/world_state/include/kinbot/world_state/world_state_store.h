#pragma once

#include <string>
#include <string_view>

#include "kinbot/common/v1/common.pb.h"
#include "kinbot/event_bus/v1/robot_event.pb.h"
#include "kinbot/world_state/v1/world_state.pb.h"

namespace kinbot::platform::world_state {

struct ApplyResult {
  bool applied = false;
  std::string message;
};

class WorldStateStore {
 public:
  WorldStateStore();

  ApplyResult ApplyEvent(const kinbot::event_bus::v1::WorldEvent& event);

  const kinbot::world_state::v1::WorldStateSnapshot& snapshot() const {
    return snapshot_;
  }

 private:
  using WorldEvent = kinbot::event_bus::v1::WorldEvent;
  using WorldEventType = kinbot::event_bus::v1::WorldEventType;

  ApplyResult ApplyPersonIdentified(const WorldEvent& event);
  ApplyResult ApplyFallSuspected(const WorldEvent& event);
  ApplyResult ApplyMedicationDue(const WorldEvent& event);
  ApplyResult ApplyTaskStateChanged(const WorldEvent& event);
  ApplyResult ApplyManualServiceState(const WorldEvent& event,
                                      std::string_view next_state);

  void FinalizeMutation(const WorldEvent& event);

  kinbot::world_state::v1::Person* UpsertActivePerson(
      const std::string& person_id, int64_t timestamp_ms,
      const WorldEvent& event);
  kinbot::world_state::v1::RiskEvent* UpsertHealthAlert(
      const std::string& risk_event_id);
  kinbot::world_state::v1::Task* UpsertTask(const std::string& task_id);

  static const std::string* FindPayload(const WorldEvent& event,
                                        std::string_view key);
  static std::string PayloadOr(const WorldEvent& event, std::string_view key,
                               std::string_view fallback);
  static kinbot::common::v1::RiskLevel ParseRiskLevel(std::string_view value);

  kinbot::world_state::v1::WorldStateSnapshot snapshot_;
};

bool ParseWorldEventType(std::string_view value,
                         kinbot::event_bus::v1::WorldEventType* out);
std::string WorldEventTypeToString(kinbot::event_bus::v1::WorldEventType type);

bool ParsePrivacyLevel(std::string_view value,
                       kinbot::common::v1::PrivacyLevel* out);
std::string PrivacyLevelToString(kinbot::common::v1::PrivacyLevel level);

}  // namespace kinbot::platform::world_state
