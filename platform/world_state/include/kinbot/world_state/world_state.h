#pragma once

#include "kinbot/world_state/types.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace kinbot {
namespace world_state {

class WorldState {
public:
    // ----- Person CRUD -----
    void add_person(const Person& p);
    Person* get_person(const std::string& person_id);
    const Person* get_person(const std::string& person_id) const;
    bool update_person(const std::string& person_id, const Person& p);
    bool remove_person(const std::string& person_id);
    std::vector<Person> get_all_persons() const;

    // ----- Task CRUD -----
    void add_task(const Task& t);
    Task* get_task(const std::string& task_id);
    const Task* get_task(const std::string& task_id) const;
    bool update_task(const std::string& task_id, const Task& t);
    bool remove_task(const std::string& task_id);
    std::vector<Task> get_all_tasks() const;

    // ----- RiskEvent CRUD -----
    void add_risk_event(const RiskEvent& re);
    RiskEvent* get_risk_event(const std::string& risk_event_id);
    const RiskEvent* get_risk_event(const std::string& risk_event_id) const;
    bool update_risk_event(const std::string& risk_event_id, const RiskEvent& re);
    bool remove_risk_event(const std::string& risk_event_id);
    std::vector<RiskEvent> get_all_risk_events() const;

    // ----- RoleBinding CRUD -----
    void add_role_binding(const RoleBinding& rb);
    RoleBinding* get_role_binding(const std::string& binding_id);
    bool remove_role_binding(const std::string& binding_id);

    // ----- Place CRUD -----
    void add_place(const Place& p);
    Place* get_place(const std::string& place_id);
    bool remove_place(const std::string& place_id);

    // ----- HealthProfile CRUD -----
    void add_health_profile(const HealthProfile& hp);
    HealthProfile* get_health_profile(const std::string& health_profile_id);
    bool remove_health_profile(const std::string& health_profile_id);

    // ----- MedicationAsset CRUD -----
    void add_medication_asset(const MedicationAsset& ma);
    MedicationAsset* get_medication_asset(const std::string& medication_id);
    bool remove_medication_asset(const std::string& medication_id);

    // ----- Object CRUD -----
    void add_object(const Object& obj);
    Object* get_object(const std::string& object_id);
    bool remove_object(const std::string& object_id);

    // ----- Household -----
    void set_household(const Household& h);
    const Household& get_household() const;

    // ----- 运行时状态 -----
    void set_battery_state(const BatteryState& bs);
    const BatteryState& get_battery_state() const;

    void set_network_state(const NetworkState& ns);
    const NetworkState& get_network_state() const;

    void set_home_mode(HomeMode mode);
    HomeMode get_home_mode() const;

    void set_medication_urgency(bool urgent);
    bool get_medication_urgency() const;

    void set_manual_service_state(const std::string& state);
    const std::string& get_manual_service_state() const;

    // ----- 快照 -----
    WorldStateSnapshot generate_snapshot() const;

    // ----- 版本 -----
    int64_t version() const;

private:
    void bump_version();

    std::unordered_map<std::string, Person> persons_;
    std::unordered_map<std::string, Task> tasks_;
    std::unordered_map<std::string, RiskEvent> risk_events_;
    std::unordered_map<std::string, RoleBinding> role_bindings_;
    std::unordered_map<std::string, Place> places_;
    std::unordered_map<std::string, HealthProfile> health_profiles_;
    std::unordered_map<std::string, MedicationAsset> medication_assets_;
    std::unordered_map<std::string, Object> objects_;
    Household household_;

    BatteryState battery_state_{};
    NetworkState network_state_{"online"};
    HomeMode home_mode_ = HomeMode::UNSPECIFIED;
    bool medication_urgency_ = false;
    std::string manual_service_state_ = "none";
    int64_t version_ = 0;
};

}  // namespace world_state
}  // namespace kinbot
