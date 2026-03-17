#include "kinbot/world_state/world_state_manager.h"

namespace kinbot {
namespace world_state {

bool WorldStateManager::handle_event(const WorldEvent& event) {
    switch (event.event_type) {
        case WorldEventType::PERSON_DETECTED:
            handle_person_detected(event);
            return true;
        case WorldEventType::PERSON_IDENTIFIED:
            handle_person_identified(event);
            return true;
        case WorldEventType::PERSON_LOST:
            handle_person_lost(event);
            return true;
        case WorldEventType::FALL_SUSPECTED:
            handle_fall_suspected(event);
            return true;
        case WorldEventType::TASK_CREATED:
            handle_task_created(event);
            return true;
        case WorldEventType::TASK_STATE_CHANGED:
            handle_task_state_changed(event);
            return true;
        case WorldEventType::MEDICATION_DUE:
            handle_medication_due(event);
            return true;
        case WorldEventType::ABNORMAL_VITAL_RECEIVED:
            handle_abnormal_vital_received(event);
            return true;
        default:
            return false;
    }
}

WorldStateSnapshot WorldStateManager::get_snapshot() const {
    return state_.generate_snapshot();
}

const WorldState& WorldStateManager::state() const {
    return state_;
}

WorldState& WorldStateManager::mutable_state() {
    return state_;
}

// ===== Event Handlers =====

void WorldStateManager::handle_person_detected(const WorldEvent& e) {
    auto pid = payload_get(e.payload, "person_id");
    if (pid.empty()) return;

    auto* existing = state_.get_person(pid);
    if (existing) {
        auto place = payload_get(e.payload, "place_id");
        if (!place.empty()) {
            existing->default_location = place;
        }
        existing->meta.updated_at_ms = e.timestamp_ms;
        // Version bump via update
        Person updated = *existing;
        state_.update_person(pid, updated);
    } else {
        Person p;
        p.person_id = pid;
        p.identity_status = "anonymous";
        p.care_priority = "normal";
        p.default_location = payload_get(e.payload, "place_id");
        p.meta.source = e.source_module;
        p.meta.created_at_ms = e.timestamp_ms;
        p.meta.updated_at_ms = e.timestamp_ms;
        auto conf_str = payload_get(e.payload, "confidence");
        if (!conf_str.empty()) {
            p.meta.confidence = std::stof(conf_str);
        }
        state_.add_person(p);
    }
}

void WorldStateManager::handle_person_identified(const WorldEvent& e) {
    auto pid = payload_get(e.payload, "person_id");
    if (pid.empty()) return;

    auto* existing = state_.get_person(pid);
    if (!existing) {
        // Create a new person first
        Person p;
        p.person_id = pid;
        p.identity_status = "anonymous";
        p.care_priority = "normal";
        p.meta.created_at_ms = e.timestamp_ms;
        p.meta.source = e.source_module;
        state_.add_person(p);
        existing = state_.get_person(pid);
    }

    existing->identity_status = "identified";
    auto name = payload_get(e.payload, "display_name");
    if (!name.empty()) existing->display_name = name;
    auto age = payload_get(e.payload, "age_group");
    if (!age.empty()) existing->age_group = age;
    auto mobility = payload_get(e.payload, "mobility_level");
    if (!mobility.empty()) existing->mobility_level = mobility;
    auto hp = payload_get(e.payload, "health_profile_id");
    if (!hp.empty()) existing->health_profile_id = hp;
    existing->meta.updated_at_ms = e.timestamp_ms;

    Person updated = *existing;
    state_.update_person(pid, updated);
}

void WorldStateManager::handle_person_lost(const WorldEvent& e) {
    auto pid = payload_get(e.payload, "person_id");
    if (pid.empty()) return;
    state_.remove_person(pid);
}

void WorldStateManager::handle_fall_suspected(const WorldEvent& e) {
    auto pid = payload_get(e.payload, "person_id");
    auto reid = payload_get(e.payload, "risk_event_id");
    if (pid.empty() || reid.empty()) return;

    RiskEvent re;
    re.risk_event_id = reid;
    re.risk_type = "fall";
    re.subject_person_id = pid;
    re.severity = parse_risk_level(payload_get(e.payload, "severity"), RiskLevel::HIGH);
    re.status = "candidate";
    state_.add_risk_event(re);

    // Escalate care priority to urgent
    auto* person = state_.get_person(pid);
    if (person) {
        person->care_priority = "urgent";
        Person updated = *person;
        state_.update_person(pid, updated);
    }
}

void WorldStateManager::handle_task_created(const WorldEvent& e) {
    auto tid = payload_get(e.payload, "task_id");
    if (tid.empty()) return;

    // Idempotent: skip if task already exists
    if (state_.get_task(tid)) return;

    Task t;
    t.task_id = tid;
    t.task_type = payload_get(e.payload, "task_type");
    t.owner_person_id = payload_get(e.payload, "owner_person_id");
    t.trigger_source = payload_get(e.payload, "trigger_source");
    t.priority = payload_get(e.payload, "priority");
    t.status = "pending";
    t.approval_status = "pending";
    state_.add_task(t);
}

void WorldStateManager::handle_task_state_changed(const WorldEvent& e) {
    auto tid = payload_get(e.payload, "task_id");
    if (tid.empty()) return;

    auto* task = state_.get_task(tid);
    if (!task) return;  // Unknown task, no-op

    auto new_status = payload_get(e.payload, "new_status");
    if (!new_status.empty()) task->status = new_status;
    auto approval = payload_get(e.payload, "approval_status");
    if (!approval.empty()) task->approval_status = approval;

    Task updated = *task;
    state_.update_task(tid, updated);
}

void WorldStateManager::handle_medication_due(const WorldEvent& e) {
    auto pid = payload_get(e.payload, "person_id");
    auto med_id = payload_get(e.payload, "medication_id");
    if (pid.empty() || med_id.empty()) return;

    state_.set_medication_urgency(true);

    auto tid = payload_get(e.payload, "task_id");
    if (tid.empty()) {
        tid = "med_task_" + med_id;
    }

    // Idempotent
    if (!state_.get_task(tid)) {
        Task t;
        t.task_id = tid;
        t.task_type = "deliver_med";
        t.owner_person_id = pid;
        t.trigger_source = "scheduled";
        t.priority = "high";
        t.status = "pending";
        t.approval_status = "pending";
        state_.add_task(t);
    }
}

void WorldStateManager::handle_abnormal_vital_received(const WorldEvent& e) {
    auto pid = payload_get(e.payload, "person_id");
    auto reid = payload_get(e.payload, "risk_event_id");
    if (pid.empty() || reid.empty()) return;

    RiskEvent re;
    re.risk_event_id = reid;
    re.risk_type = "abnormal_vital";
    re.subject_person_id = pid;
    re.severity = parse_risk_level(payload_get(e.payload, "severity"), RiskLevel::MEDIUM);
    re.status = "candidate";
    state_.add_risk_event(re);

    // Monotonic care priority escalation: normal -> focused -> urgent
    auto* person = state_.get_person(pid);
    if (person) {
        if (person->care_priority == "normal") {
            person->care_priority = "focused";
        } else if (person->care_priority == "focused") {
            person->care_priority = "urgent";
        }
        // "urgent" stays as is
        Person updated = *person;
        state_.update_person(pid, updated);
    }
}

// ===== Helpers =====

std::string WorldStateManager::payload_get(
    const std::unordered_map<std::string, std::string>& payload,
    const std::string& key) {
    auto it = payload.find(key);
    return it != payload.end() ? it->second : "";
}

RiskLevel WorldStateManager::parse_risk_level(const std::string& s, RiskLevel default_level) {
    if (s == "LOW") return RiskLevel::LOW;
    if (s == "MEDIUM") return RiskLevel::MEDIUM;
    if (s == "HIGH") return RiskLevel::HIGH;
    if (s == "CRITICAL") return RiskLevel::CRITICAL;
    return default_level;
}

}  // namespace world_state
}  // namespace kinbot
