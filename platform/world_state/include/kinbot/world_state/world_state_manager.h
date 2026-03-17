#pragma once

#include "kinbot/world_state/types.h"
#include "kinbot/world_state/world_state.h"

#include <string>
#include <unordered_map>

namespace kinbot {
namespace world_state {

class WorldStateManager {
public:
    WorldStateManager() = default;

    // Primary entry point. Returns true if the event type was handled,
    // false if the event type is not yet implemented.
    bool handle_event(const WorldEvent& event);

    // Delegates to WorldState::generate_snapshot().
    WorldStateSnapshot get_snapshot() const;

    // Direct read-only access to underlying state (for tests and queries).
    const WorldState& state() const;

    // Mutable access (for tests).
    WorldState& mutable_state();

private:
    WorldState state_;

    // Per-event handlers (Phase 0: 8 handlers)
    void handle_person_detected(const WorldEvent& e);
    void handle_person_identified(const WorldEvent& e);
    void handle_person_lost(const WorldEvent& e);
    void handle_fall_suspected(const WorldEvent& e);
    void handle_task_created(const WorldEvent& e);
    void handle_task_state_changed(const WorldEvent& e);
    void handle_medication_due(const WorldEvent& e);
    void handle_abnormal_vital_received(const WorldEvent& e);

    // Helper to safely read a payload key, returning empty string if absent.
    static std::string payload_get(
        const std::unordered_map<std::string, std::string>& payload,
        const std::string& key);

    // Parse a risk level string ("LOW", "MEDIUM", "HIGH", "CRITICAL") to enum.
    static RiskLevel parse_risk_level(const std::string& s, RiskLevel default_level);
};

}  // namespace world_state
}  // namespace kinbot
