#include "kinbot/world_state/world_state.h"

#include <chrono>

namespace kinbot {
namespace world_state {

// ===== Person =====

void WorldState::add_person(const Person& p) {
    persons_[p.person_id] = p;
    bump_version();
}

Person* WorldState::get_person(const std::string& person_id) {
    auto it = persons_.find(person_id);
    return it != persons_.end() ? &it->second : nullptr;
}

const Person* WorldState::get_person(const std::string& person_id) const {
    auto it = persons_.find(person_id);
    return it != persons_.end() ? &it->second : nullptr;
}

bool WorldState::update_person(const std::string& person_id, const Person& p) {
    auto it = persons_.find(person_id);
    if (it == persons_.end()) return false;
    it->second = p;
    bump_version();
    return true;
}

bool WorldState::remove_person(const std::string& person_id) {
    if (persons_.erase(person_id) > 0) {
        bump_version();
        return true;
    }
    return false;
}

std::vector<Person> WorldState::get_all_persons() const {
    std::vector<Person> result;
    result.reserve(persons_.size());
    for (const auto& [id, p] : persons_) {
        result.push_back(p);
    }
    return result;
}

// ===== Task =====

void WorldState::add_task(const Task& t) {
    tasks_[t.task_id] = t;
    bump_version();
}

Task* WorldState::get_task(const std::string& task_id) {
    auto it = tasks_.find(task_id);
    return it != tasks_.end() ? &it->second : nullptr;
}

const Task* WorldState::get_task(const std::string& task_id) const {
    auto it = tasks_.find(task_id);
    return it != tasks_.end() ? &it->second : nullptr;
}

bool WorldState::update_task(const std::string& task_id, const Task& t) {
    auto it = tasks_.find(task_id);
    if (it == tasks_.end()) return false;
    it->second = t;
    bump_version();
    return true;
}

bool WorldState::remove_task(const std::string& task_id) {
    if (tasks_.erase(task_id) > 0) {
        bump_version();
        return true;
    }
    return false;
}

std::vector<Task> WorldState::get_all_tasks() const {
    std::vector<Task> result;
    result.reserve(tasks_.size());
    for (const auto& [id, t] : tasks_) {
        result.push_back(t);
    }
    return result;
}

// ===== RiskEvent =====

void WorldState::add_risk_event(const RiskEvent& re) {
    risk_events_[re.risk_event_id] = re;
    bump_version();
}

RiskEvent* WorldState::get_risk_event(const std::string& risk_event_id) {
    auto it = risk_events_.find(risk_event_id);
    return it != risk_events_.end() ? &it->second : nullptr;
}

const RiskEvent* WorldState::get_risk_event(const std::string& risk_event_id) const {
    auto it = risk_events_.find(risk_event_id);
    return it != risk_events_.end() ? &it->second : nullptr;
}

bool WorldState::update_risk_event(const std::string& risk_event_id, const RiskEvent& re) {
    auto it = risk_events_.find(risk_event_id);
    if (it == risk_events_.end()) return false;
    it->second = re;
    bump_version();
    return true;
}

bool WorldState::remove_risk_event(const std::string& risk_event_id) {
    if (risk_events_.erase(risk_event_id) > 0) {
        bump_version();
        return true;
    }
    return false;
}

std::vector<RiskEvent> WorldState::get_all_risk_events() const {
    std::vector<RiskEvent> result;
    result.reserve(risk_events_.size());
    for (const auto& [id, re] : risk_events_) {
        result.push_back(re);
    }
    return result;
}

// ===== RoleBinding =====

void WorldState::add_role_binding(const RoleBinding& rb) {
    role_bindings_[rb.binding_id] = rb;
    bump_version();
}

RoleBinding* WorldState::get_role_binding(const std::string& binding_id) {
    auto it = role_bindings_.find(binding_id);
    return it != role_bindings_.end() ? &it->second : nullptr;
}

bool WorldState::remove_role_binding(const std::string& binding_id) {
    if (role_bindings_.erase(binding_id) > 0) {
        bump_version();
        return true;
    }
    return false;
}

// ===== Place =====

void WorldState::add_place(const Place& p) {
    places_[p.place_id] = p;
    bump_version();
}

Place* WorldState::get_place(const std::string& place_id) {
    auto it = places_.find(place_id);
    return it != places_.end() ? &it->second : nullptr;
}

bool WorldState::remove_place(const std::string& place_id) {
    if (places_.erase(place_id) > 0) {
        bump_version();
        return true;
    }
    return false;
}

// ===== HealthProfile =====

void WorldState::add_health_profile(const HealthProfile& hp) {
    health_profiles_[hp.health_profile_id] = hp;
    bump_version();
}

HealthProfile* WorldState::get_health_profile(const std::string& health_profile_id) {
    auto it = health_profiles_.find(health_profile_id);
    return it != health_profiles_.end() ? &it->second : nullptr;
}

bool WorldState::remove_health_profile(const std::string& health_profile_id) {
    if (health_profiles_.erase(health_profile_id) > 0) {
        bump_version();
        return true;
    }
    return false;
}

// ===== MedicationAsset =====

void WorldState::add_medication_asset(const MedicationAsset& ma) {
    medication_assets_[ma.medication_id] = ma;
    bump_version();
}

MedicationAsset* WorldState::get_medication_asset(const std::string& medication_id) {
    auto it = medication_assets_.find(medication_id);
    return it != medication_assets_.end() ? &it->second : nullptr;
}

bool WorldState::remove_medication_asset(const std::string& medication_id) {
    if (medication_assets_.erase(medication_id) > 0) {
        bump_version();
        return true;
    }
    return false;
}

// ===== Object =====

void WorldState::add_object(const Object& obj) {
    objects_[obj.object_id] = obj;
    bump_version();
}

Object* WorldState::get_object(const std::string& object_id) {
    auto it = objects_.find(object_id);
    return it != objects_.end() ? &it->second : nullptr;
}

bool WorldState::remove_object(const std::string& object_id) {
    if (objects_.erase(object_id) > 0) {
        bump_version();
        return true;
    }
    return false;
}

// ===== Household =====

void WorldState::set_household(const Household& h) {
    household_ = h;
    bump_version();
}

const Household& WorldState::get_household() const {
    return household_;
}

// ===== 运行时状态 =====

void WorldState::set_battery_state(const BatteryState& bs) {
    battery_state_ = bs;
    bump_version();
}

const BatteryState& WorldState::get_battery_state() const {
    return battery_state_;
}

void WorldState::set_network_state(const NetworkState& ns) {
    network_state_ = ns;
    bump_version();
}

const NetworkState& WorldState::get_network_state() const {
    return network_state_;
}

void WorldState::set_home_mode(HomeMode mode) {
    home_mode_ = mode;
    bump_version();
}

HomeMode WorldState::get_home_mode() const {
    return home_mode_;
}

void WorldState::set_medication_urgency(bool urgent) {
    medication_urgency_ = urgent;
    bump_version();
}

bool WorldState::get_medication_urgency() const {
    return medication_urgency_;
}

void WorldState::set_manual_service_state(const std::string& state) {
    manual_service_state_ = state;
    bump_version();
}

const std::string& WorldState::get_manual_service_state() const {
    return manual_service_state_;
}

// ===== 快照 =====

WorldStateSnapshot WorldState::generate_snapshot() const {
    DecisionContextSnapshot ctx;
    auto now = std::chrono::system_clock::now();
    ctx.timestamp_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    ctx.active_persons = get_all_persons();

    for (const auto& [id, re] : risk_events_) {
        if (re.status != "closed") {
            ctx.health_alerts.push_back(re);
        }
    }

    for (const auto& [id, t] : tasks_) {
        if (t.status == "pending" || t.status == "executing" || t.status == "blocked") {
            ctx.active_tasks.push_back(t);
        }
    }

    ctx.network_state = network_state_;
    ctx.battery_state = battery_state_;
    ctx.home_mode = home_mode_;
    ctx.medication_urgency = medication_urgency_;
    ctx.manual_service_state = manual_service_state_;

    WorldStateSnapshot snap;
    snap.snapshot_state = ctx;
    snap.snapshot_version = version_;
    return snap;
}

// ===== 版本 =====

int64_t WorldState::version() const {
    return version_;
}

void WorldState::bump_version() {
    ++version_;
}

}  // namespace world_state
}  // namespace kinbot
