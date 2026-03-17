#include "kinbot/world_state/world_state_manager.h"

#include <gtest/gtest.h>

using namespace kinbot::world_state;

namespace {

WorldEvent make_event(WorldEventType type,
                      std::unordered_map<std::string, std::string> payload,
                      int64_t ts = 1000) {
    WorldEvent e;
    e.event_id = "evt_test";
    e.event_type = type;
    e.timestamp_ms = ts;
    e.source_module = "test";
    e.payload = std::move(payload);
    return e;
}

}  // namespace

// ===== PERSON_DETECTED =====

TEST(WorldStateManagerTest, PersonDetectedCreatesAnonymous) {
    WorldStateManager mgr;
    auto e = make_event(WorldEventType::PERSON_DETECTED,
                        {{"person_id", "p1"}, {"place_id", "kitchen"}});
    EXPECT_TRUE(mgr.handle_event(e));

    auto* p = mgr.state().get_person("p1");
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->identity_status, "anonymous");
    EXPECT_EQ(p->default_location, "kitchen");
    EXPECT_EQ(p->care_priority, "normal");
    EXPECT_EQ(p->meta.created_at_ms, 1000);
}

TEST(WorldStateManagerTest, PersonDetectedIdempotent) {
    WorldStateManager mgr;
    auto e1 = make_event(WorldEventType::PERSON_DETECTED,
                         {{"person_id", "p1"}, {"place_id", "kitchen"}}, 1000);
    auto e2 = make_event(WorldEventType::PERSON_DETECTED,
                         {{"person_id", "p1"}, {"place_id", "bedroom"}}, 2000);
    mgr.handle_event(e1);
    mgr.handle_event(e2);

    // Still one person, location updated
    EXPECT_EQ(mgr.state().get_all_persons().size(), 1u);
    EXPECT_EQ(mgr.state().get_person("p1")->default_location, "bedroom");
    EXPECT_EQ(mgr.state().get_person("p1")->meta.updated_at_ms, 2000);
}

TEST(WorldStateManagerTest, PersonDetectedWithConfidence) {
    WorldStateManager mgr;
    auto e = make_event(WorldEventType::PERSON_DETECTED,
                        {{"person_id", "p1"}, {"confidence", "0.85"}});
    mgr.handle_event(e);
    EXPECT_FLOAT_EQ(mgr.state().get_person("p1")->meta.confidence, 0.85f);
}

// ===== PERSON_IDENTIFIED =====

TEST(WorldStateManagerTest, PersonIdentifiedUpdatesStatus) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::PERSON_DETECTED,
                                {{"person_id", "p1"}}));
    mgr.handle_event(make_event(WorldEventType::PERSON_IDENTIFIED,
                                {{"person_id", "p1"},
                                 {"display_name", "Grandma Wang"},
                                 {"age_group", "elder"},
                                 {"mobility_level", "limited"}}));

    auto* p = mgr.state().get_person("p1");
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->identity_status, "identified");
    EXPECT_EQ(p->display_name, "Grandma Wang");
    EXPECT_EQ(p->age_group, "elder");
    EXPECT_EQ(p->mobility_level, "limited");
}

TEST(WorldStateManagerTest, PersonIdentifiedWithoutPriorDetect) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::PERSON_IDENTIFIED,
                                {{"person_id", "p1"},
                                 {"display_name", "Bob"}}));

    auto* p = mgr.state().get_person("p1");
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->identity_status, "identified");
    EXPECT_EQ(p->display_name, "Bob");
}

// ===== PERSON_LOST =====

TEST(WorldStateManagerTest, PersonLostRemoves) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::PERSON_DETECTED,
                                {{"person_id", "p1"}}));
    mgr.handle_event(make_event(WorldEventType::PERSON_LOST,
                                {{"person_id", "p1"}}));

    EXPECT_EQ(mgr.state().get_person("p1"), nullptr);
    EXPECT_TRUE(mgr.get_snapshot().snapshot_state.active_persons.empty());
}

TEST(WorldStateManagerTest, PersonLostNonExistentNoOp) {
    WorldStateManager mgr;
    auto v_before = mgr.state().version();
    mgr.handle_event(make_event(WorldEventType::PERSON_LOST,
                                {{"person_id", "ghost"}}));
    // No crash, version unchanged (remove_person returns false => no bump)
    EXPECT_EQ(mgr.state().version(), v_before);
}

// ===== FALL_SUSPECTED =====

TEST(WorldStateManagerTest, FallSuspectedCreatesRiskEvent) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::FALL_SUSPECTED,
                                {{"person_id", "p1"},
                                 {"risk_event_id", "re1"}}));

    auto* re = mgr.state().get_risk_event("re1");
    ASSERT_NE(re, nullptr);
    EXPECT_EQ(re->risk_type, "fall");
    EXPECT_EQ(re->severity, RiskLevel::HIGH);  // default
    EXPECT_EQ(re->status, "candidate");
    EXPECT_EQ(re->subject_person_id, "p1");
}

TEST(WorldStateManagerTest, FallSuspectedWithExplicitSeverity) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::FALL_SUSPECTED,
                                {{"person_id", "p1"},
                                 {"risk_event_id", "re1"},
                                 {"severity", "CRITICAL"}}));

    EXPECT_EQ(mgr.state().get_risk_event("re1")->severity, RiskLevel::CRITICAL);
}

TEST(WorldStateManagerTest, FallSuspectedEscalatesCarePriority) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::PERSON_DETECTED,
                                {{"person_id", "p1"}}));
    EXPECT_EQ(mgr.state().get_person("p1")->care_priority, "normal");

    mgr.handle_event(make_event(WorldEventType::FALL_SUSPECTED,
                                {{"person_id", "p1"},
                                 {"risk_event_id", "re1"}}));
    EXPECT_EQ(mgr.state().get_person("p1")->care_priority, "urgent");
}

TEST(WorldStateManagerTest, FallSuspectedInSnapshot) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::FALL_SUSPECTED,
                                {{"person_id", "p1"},
                                 {"risk_event_id", "re1"}}));

    auto snap = mgr.get_snapshot();
    EXPECT_EQ(snap.snapshot_state.health_alerts.size(), 1u);
    EXPECT_EQ(snap.snapshot_state.health_alerts[0].risk_type, "fall");
}

// ===== TASK_CREATED =====

TEST(WorldStateManagerTest, TaskCreatedAddsTask) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::TASK_CREATED,
                                {{"task_id", "t1"},
                                 {"task_type", "deliver_med"},
                                 {"priority", "high"}}));

    auto* t = mgr.state().get_task("t1");
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->task_type, "deliver_med");
    EXPECT_EQ(t->status, "pending");
    EXPECT_EQ(t->approval_status, "pending");
    EXPECT_EQ(t->priority, "high");
}

TEST(WorldStateManagerTest, TaskCreatedIdempotent) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::TASK_CREATED,
                                {{"task_id", "t1"},
                                 {"task_type", "deliver_med"}}));
    auto v = mgr.state().version();

    // Second event with same task_id is a no-op
    mgr.handle_event(make_event(WorldEventType::TASK_CREATED,
                                {{"task_id", "t1"},
                                 {"task_type", "companion"}}));
    EXPECT_EQ(mgr.state().version(), v);
    EXPECT_EQ(mgr.state().get_task("t1")->task_type, "deliver_med");
}

// ===== TASK_STATE_CHANGED =====

TEST(WorldStateManagerTest, TaskStateChanged) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::TASK_CREATED,
                                {{"task_id", "t1"},
                                 {"task_type", "deliver_med"}}));
    mgr.handle_event(make_event(WorldEventType::TASK_STATE_CHANGED,
                                {{"task_id", "t1"},
                                 {"new_status", "executing"},
                                 {"approval_status", "approved"}}));

    auto* t = mgr.state().get_task("t1");
    EXPECT_EQ(t->status, "executing");
    EXPECT_EQ(t->approval_status, "approved");
}

TEST(WorldStateManagerTest, TaskStateChangedUnknownTask) {
    WorldStateManager mgr;
    auto v = mgr.state().version();
    // handle_event returns true (event type recognized) but does nothing
    EXPECT_TRUE(mgr.handle_event(make_event(WorldEventType::TASK_STATE_CHANGED,
                                            {{"task_id", "unknown"},
                                             {"new_status", "executing"}})));
    EXPECT_EQ(mgr.state().version(), v);
}

TEST(WorldStateManagerTest, TaskStateChangedCompletedNotInActiveSnapshot) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::TASK_CREATED,
                                {{"task_id", "t1"},
                                 {"task_type", "deliver_med"}}));
    mgr.handle_event(make_event(WorldEventType::TASK_STATE_CHANGED,
                                {{"task_id", "t1"},
                                 {"new_status", "completed"}}));

    auto snap = mgr.get_snapshot();
    EXPECT_TRUE(snap.snapshot_state.active_tasks.empty());
}

// ===== MEDICATION_DUE =====

TEST(WorldStateManagerTest, MedicationDueCreatesTaskAndSetsUrgency) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::MEDICATION_DUE,
                                {{"person_id", "p1"},
                                 {"medication_id", "med1"},
                                 {"task_id", "t_med1"}}));

    auto snap = mgr.get_snapshot();
    EXPECT_TRUE(snap.snapshot_state.medication_urgency);

    auto* t = mgr.state().get_task("t_med1");
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->task_type, "deliver_med");
    EXPECT_EQ(t->owner_person_id, "p1");
    EXPECT_EQ(t->priority, "high");
    EXPECT_EQ(t->status, "pending");
}

TEST(WorldStateManagerTest, MedicationDueAutoGeneratesTaskId) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::MEDICATION_DUE,
                                {{"person_id", "p1"},
                                 {"medication_id", "med1"}}));

    auto* t = mgr.state().get_task("med_task_med1");
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->task_type, "deliver_med");
}

// ===== ABNORMAL_VITAL_RECEIVED =====

TEST(WorldStateManagerTest, AbnormalVitalCreatesRiskEvent) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::ABNORMAL_VITAL_RECEIVED,
                                {{"person_id", "p1"},
                                 {"risk_event_id", "re_v1"}}));

    auto* re = mgr.state().get_risk_event("re_v1");
    ASSERT_NE(re, nullptr);
    EXPECT_EQ(re->risk_type, "abnormal_vital");
    EXPECT_EQ(re->severity, RiskLevel::MEDIUM);  // default
    EXPECT_EQ(re->status, "candidate");
}

TEST(WorldStateManagerTest, AbnormalVitalEscalatesCarePriorityMonotonically) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::PERSON_DETECTED,
                                {{"person_id", "p1"}}));
    EXPECT_EQ(mgr.state().get_person("p1")->care_priority, "normal");

    // First: normal -> focused
    mgr.handle_event(make_event(WorldEventType::ABNORMAL_VITAL_RECEIVED,
                                {{"person_id", "p1"},
                                 {"risk_event_id", "re1"}}));
    EXPECT_EQ(mgr.state().get_person("p1")->care_priority, "focused");

    // Second: focused -> urgent
    mgr.handle_event(make_event(WorldEventType::ABNORMAL_VITAL_RECEIVED,
                                {{"person_id", "p1"},
                                 {"risk_event_id", "re2"}}));
    EXPECT_EQ(mgr.state().get_person("p1")->care_priority, "urgent");

    // Third: urgent stays urgent (ceiling)
    mgr.handle_event(make_event(WorldEventType::ABNORMAL_VITAL_RECEIVED,
                                {{"person_id", "p1"},
                                 {"risk_event_id", "re3"}}));
    EXPECT_EQ(mgr.state().get_person("p1")->care_priority, "urgent");
}

// ===== Unhandled event =====

TEST(WorldStateManagerTest, UnhandledEventReturnsFalse) {
    WorldStateManager mgr;
    auto e = make_event(WorldEventType::VOICE_COMMAND_RECEIVED,
                        {{"command", "hello"}});
    EXPECT_FALSE(mgr.handle_event(e));
}

// ===== Composite scenario =====

TEST(WorldStateManagerTest, FullScenarioElderFallAndMedication) {
    WorldStateManager mgr;

    // Step 1: Person detected
    mgr.handle_event(make_event(WorldEventType::PERSON_DETECTED,
                                {{"person_id", "elder1"},
                                 {"place_id", "living_room"}}, 1000));

    // Step 2: Person identified as elder
    mgr.handle_event(make_event(WorldEventType::PERSON_IDENTIFIED,
                                {{"person_id", "elder1"},
                                 {"display_name", "Grandma Wang"},
                                 {"age_group", "elder"},
                                 {"mobility_level", "limited"}}, 2000));

    // Step 3: Fall suspected
    mgr.handle_event(make_event(WorldEventType::FALL_SUSPECTED,
                                {{"person_id", "elder1"},
                                 {"risk_event_id", "re_fall1"}}, 3000));

    // Step 4: Medication due
    mgr.handle_event(make_event(WorldEventType::MEDICATION_DUE,
                                {{"person_id", "elder1"},
                                 {"medication_id", "med_bp"}}, 4000));

    auto snap = mgr.get_snapshot();

    // 1 active person, identified as elder, care_priority urgent
    EXPECT_EQ(snap.snapshot_state.active_persons.size(), 1u);
    EXPECT_EQ(snap.snapshot_state.active_persons[0].display_name, "Grandma Wang");
    EXPECT_EQ(snap.snapshot_state.active_persons[0].age_group, "elder");
    EXPECT_EQ(snap.snapshot_state.active_persons[0].care_priority, "urgent");

    // 1 risk event (fall)
    EXPECT_EQ(snap.snapshot_state.health_alerts.size(), 1u);
    EXPECT_EQ(snap.snapshot_state.health_alerts[0].risk_type, "fall");

    // 1 active task (deliver_med from MEDICATION_DUE)
    EXPECT_EQ(snap.snapshot_state.active_tasks.size(), 1u);
    EXPECT_EQ(snap.snapshot_state.active_tasks[0].task_type, "deliver_med");

    // Medication urgency set
    EXPECT_TRUE(snap.snapshot_state.medication_urgency);
}

// ===== Missing required payload =====

TEST(WorldStateManagerTest, PersonDetectedMissingPersonIdNoOp) {
    WorldStateManager mgr;
    mgr.handle_event(make_event(WorldEventType::PERSON_DETECTED, {}));
    EXPECT_TRUE(mgr.state().get_all_persons().empty());
}

TEST(WorldStateManagerTest, FallSuspectedMissingFieldsNoOp) {
    WorldStateManager mgr;
    // Missing risk_event_id
    mgr.handle_event(make_event(WorldEventType::FALL_SUSPECTED,
                                {{"person_id", "p1"}}));
    EXPECT_TRUE(mgr.state().get_all_risk_events().empty());
}
