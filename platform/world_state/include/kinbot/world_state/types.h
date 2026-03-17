#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace kinbot {
namespace world_state {

// ===== 通用枚举（镜像 common.proto）=====

enum class PrivacyLevel {
    UNSPECIFIED = 0,
    PUBLIC_RUNTIME,
    PERSONAL_SENSITIVE,
    BIOMETRIC_SENSITIVE,
    MEDICAL_SENSITIVE,
};

enum class RiskLevel {
    UNSPECIFIED = 0,
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL,
};

enum class Role {
    UNSPECIFIED = 0,
    ELDER,
    CHILD,
    CAREGIVER,
    VISITOR,
    ROBOT_SYSTEM,
    OPS_SEAT,
    THIRD_PARTY_PLATFORM,
};

enum class HomeMode {
    UNSPECIFIED = 0,
    DAYTIME,
    NIGHTTIME,
    AWAY,
    RESTING,
    ANOMALY_ACTIVE,
};

// ===== 事件类型（镜像 robot_event.proto）=====

enum class WorldEventType {
    UNSPECIFIED = 0,
    PERSON_DETECTED,
    PERSON_IDENTIFIED,
    PERSON_LOST,
    VOICE_COMMAND_RECEIVED,
    FALL_SUSPECTED,
    ABNORMAL_VITAL_RECEIVED,
    WEARABLE_SIGNAL_RECEIVED,
    MEDICATION_DUE,
    MEDICATION_LOW_STOCK,
    TASK_CREATED,
    TASK_STATE_CHANGED,
    AUTHORIZATION_CHANGED,
    CAREGIVER_MODE_ENABLED,
    COMPARTMENT_OPENED,
    COMPARTMENT_CLOSED,
    COMPARTMENT_BLOCKED,
    ROBOT_NEAR_ELDER,
    ROBOT_DELIVERY_COMPLETED,
    MANUAL_SERVICE_REQUESTED,
    MANUAL_SERVICE_CONNECTED,
    MANUAL_SERVICE_TRANSFERRED,
    MANUAL_SERVICE_TIMEOUT,
};

// ===== WorldEvent（镜像 robot_event.proto）=====

struct WorldEvent {
    std::string event_id;
    WorldEventType event_type = WorldEventType::UNSPECIFIED;
    int64_t timestamp_ms = 0;
    std::string source_module;
    std::unordered_map<std::string, std::string> payload;
    PrivacyLevel privacy_level = PrivacyLevel::UNSPECIFIED;
};

// ===== 实体元数据（镜像 common.proto EntityMeta）=====

struct EntityMeta {
    std::string id;
    std::string type;
    uint64_t version = 0;
    int64_t created_at_ms = 0;
    int64_t updated_at_ms = 0;
    std::string source;
    float confidence = 0.0f;
    PrivacyLevel privacy_level = PrivacyLevel::UNSPECIFIED;
};

// ===== 9 类一级实体（镜像 world_state.proto）=====

struct Person {
    EntityMeta meta;
    std::string person_id;
    std::string identity_status;  // "identified", "anonymous", "uncertain"
    std::string display_name;
    std::string age_group;        // "elder", "adult", "child"
    std::string mobility_level;   // "normal", "limited", "assisted"
    std::string health_profile_id;
    std::string default_location;
    std::string care_priority;    // "normal", "focused", "urgent"
};

struct RoleBinding {
    std::string binding_id;
    std::string person_id;
    Role role = Role::UNSPECIFIED;
    std::vector<std::string> auth_scope;
    std::string delegated_by;
    int32_t priority_rank = 0;
};

struct CareNetworkEntry {
    std::string name;
    std::string service_type;  // "family", "community", "hospital", "pharmacy", "delivery"
    std::string contact_ref;
    std::string responsibility_boundary;
};

struct Household {
    std::string household_id;
    std::vector<std::string> member_ids;
    std::vector<CareNetworkEntry> care_network;
    HomeMode home_mode = HomeMode::UNSPECIFIED;
};

struct Place {
    std::string place_id;
    std::string category;        // "room", "corridor", "threshold", "charging_point", "risk_zone"
    std::string topology_parent;
    std::string navigability;    // "reachable", "restricted", "dangerous"
    std::string care_relevance;  // "normal", "high"
};

struct Object {
    std::string object_id;
    std::string category;
    std::string current_place_id;
    std::string ownership;
    std::unordered_map<std::string, std::string> state;
    std::vector<std::string> care_tags;
};

struct HealthProfile {
    std::string health_profile_id;
    std::string person_id;
    std::vector<std::string> chronic_conditions;
    std::vector<std::string> allergies;
    std::vector<std::string> contraindications;
    std::vector<std::string> medication_plan_ids;
    std::vector<std::string> device_binding_refs;
};

struct MedicationAsset {
    std::string medication_id;
    std::string owner_person_id;
    std::string name;
    std::string form;              // "tablet", "capsule", "liquid"
    std::string storage_place_id;
    int32_t remaining_count = 0;
    std::string expiry_date;
    std::string prescription_ref;
    std::string delivery_status;   // "not_ordered", "in_transit", "delivered"
};

struct Task {
    std::string task_id;
    std::string task_type;         // "companion", "remind", "patrol", "find_person", "deliver_med"
    std::string owner_person_id;
    std::string trigger_source;    // "user_request", "system_detect", "scheduled", "cloud_trigger"
    std::string priority;
    std::string status;            // "pending", "executing", "blocked", "completed", "failed"
    std::string approval_status;   // "pending", "approved", "rejected"
};

struct RiskEvent {
    std::string risk_event_id;
    std::string risk_type;         // "fall", "unconscious_suspect", "collision", "privacy"
    std::string subject_person_id;
    RiskLevel severity = RiskLevel::UNSPECIFIED;
    std::string status;            // "candidate", "confirmed", "handling", "closed"
    std::string linked_task_id;
};

// ===== 运行时状态（镜像 world_state.proto）=====

struct BatteryState {
    float level_percent = 100.0f;
    bool charging = false;
    bool return_to_charge_needed = false;
};

struct NetworkState {
    std::string status = "online";  // "online", "weak", "offline"
};

struct WearableFreshnessState {
    std::string device_id;
    std::string acquisition_mode;  // "broadcast", "sdk_bound", "questionnaire_driven"
    int64_t last_reading_ms = 0;
    bool stale = false;
};

// ===== 快照（镜像 world_state.proto）=====

struct DecisionContextSnapshot {
    int64_t timestamp_ms = 0;
    std::vector<Person> active_persons;
    std::vector<RiskEvent> health_alerts;
    std::vector<Task> active_tasks;
    NetworkState network_state;
    BatteryState battery_state;
    HomeMode home_mode = HomeMode::UNSPECIFIED;
    std::vector<WearableFreshnessState> wearable_freshness;
    bool medication_urgency = false;
    std::string manual_service_state = "none";  // "none", "requested", "connected", "timeout"
};

struct WorldStateSnapshot {
    DecisionContextSnapshot snapshot_state;
    int64_t snapshot_version = 0;
};

}  // namespace world_state
}  // namespace kinbot
