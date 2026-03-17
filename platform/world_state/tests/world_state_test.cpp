#include "kinbot/world_state/world_state.h"

#include <gtest/gtest.h>

using namespace kinbot::world_state;

// ===== Person CRUD =====

TEST(WorldStateTest, AddAndGetPerson) {
    WorldState ws;
    Person p;
    p.person_id = "p1";
    p.display_name = "Alice";
    p.identity_status = "identified";
    p.care_priority = "normal";

    ws.add_person(p);
    const auto* got = ws.get_person("p1");
    ASSERT_NE(got, nullptr);
    EXPECT_EQ(got->person_id, "p1");
    EXPECT_EQ(got->display_name, "Alice");
    EXPECT_EQ(got->identity_status, "identified");
}

TEST(WorldStateTest, UpdatePerson) {
    WorldState ws;
    Person p;
    p.person_id = "p1";
    p.care_priority = "normal";
    ws.add_person(p);

    Person updated = p;
    updated.care_priority = "urgent";
    EXPECT_TRUE(ws.update_person("p1", updated));
    EXPECT_EQ(ws.get_person("p1")->care_priority, "urgent");
}

TEST(WorldStateTest, UpdateNonExistentPersonFails) {
    WorldState ws;
    Person p;
    p.person_id = "p1";
    EXPECT_FALSE(ws.update_person("p1", p));
}

TEST(WorldStateTest, RemovePerson) {
    WorldState ws;
    Person p;
    p.person_id = "p1";
    ws.add_person(p);

    EXPECT_TRUE(ws.remove_person("p1"));
    EXPECT_EQ(ws.get_person("p1"), nullptr);
}

TEST(WorldStateTest, RemoveNonExistentPerson) {
    WorldState ws;
    EXPECT_FALSE(ws.remove_person("no_such_id"));
}

TEST(WorldStateTest, ListPersons) {
    WorldState ws;
    for (int i = 0; i < 3; ++i) {
        Person p;
        p.person_id = "p" + std::to_string(i);
        ws.add_person(p);
    }
    EXPECT_EQ(ws.get_all_persons().size(), 3u);
}

TEST(WorldStateTest, AddDuplicatePersonOverwrites) {
    // Current implementation: add_person uses operator[], so duplicate overwrites
    WorldState ws;
    Person p;
    p.person_id = "p1";
    p.display_name = "First";
    ws.add_person(p);

    Person p2;
    p2.person_id = "p1";
    p2.display_name = "Second";
    ws.add_person(p2);

    EXPECT_EQ(ws.get_person("p1")->display_name, "Second");
    EXPECT_EQ(ws.get_all_persons().size(), 1u);
}

// ===== Task CRUD =====

TEST(WorldStateTest, TaskCrud) {
    WorldState ws;
    Task t;
    t.task_id = "t1";
    t.task_type = "deliver_med";
    t.status = "pending";
    ws.add_task(t);

    auto* got = ws.get_task("t1");
    ASSERT_NE(got, nullptr);
    EXPECT_EQ(got->task_type, "deliver_med");

    Task updated = *got;
    updated.status = "executing";
    EXPECT_TRUE(ws.update_task("t1", updated));
    EXPECT_EQ(ws.get_task("t1")->status, "executing");

    EXPECT_TRUE(ws.remove_task("t1"));
    EXPECT_EQ(ws.get_task("t1"), nullptr);
}

// ===== RiskEvent CRUD =====

TEST(WorldStateTest, RiskEventCrud) {
    WorldState ws;
    RiskEvent re;
    re.risk_event_id = "re1";
    re.risk_type = "fall";
    re.severity = RiskLevel::HIGH;
    re.status = "candidate";
    ws.add_risk_event(re);

    auto* got = ws.get_risk_event("re1");
    ASSERT_NE(got, nullptr);
    EXPECT_EQ(got->risk_type, "fall");
    EXPECT_EQ(got->severity, RiskLevel::HIGH);

    RiskEvent updated = *got;
    updated.status = "closed";
    EXPECT_TRUE(ws.update_risk_event("re1", updated));
    EXPECT_EQ(ws.get_risk_event("re1")->status, "closed");

    EXPECT_TRUE(ws.remove_risk_event("re1"));
    EXPECT_EQ(ws.get_risk_event("re1"), nullptr);
}

// ===== Snapshot =====

TEST(WorldStateTest, SnapshotActivePersons) {
    WorldState ws;
    Person p1; p1.person_id = "p1";
    Person p2; p2.person_id = "p2";
    ws.add_person(p1);
    ws.add_person(p2);

    auto snap = ws.generate_snapshot();
    EXPECT_EQ(snap.snapshot_state.active_persons.size(), 2u);
}

TEST(WorldStateTest, SnapshotActiveTasksFiltering) {
    WorldState ws;
    Task t1; t1.task_id = "t1"; t1.status = "pending";
    Task t2; t2.task_id = "t2"; t2.status = "executing";
    Task t3; t3.task_id = "t3"; t3.status = "completed";
    Task t4; t4.task_id = "t4"; t4.status = "blocked";
    ws.add_task(t1);
    ws.add_task(t2);
    ws.add_task(t3);
    ws.add_task(t4);

    auto snap = ws.generate_snapshot();
    // pending, executing, blocked are active; completed is not
    EXPECT_EQ(snap.snapshot_state.active_tasks.size(), 3u);
}

TEST(WorldStateTest, SnapshotHealthAlertsFiltering) {
    WorldState ws;
    RiskEvent re1; re1.risk_event_id = "re1"; re1.status = "candidate";
    RiskEvent re2; re2.risk_event_id = "re2"; re2.status = "handling";
    RiskEvent re3; re3.risk_event_id = "re3"; re3.status = "closed";
    ws.add_risk_event(re1);
    ws.add_risk_event(re2);
    ws.add_risk_event(re3);

    auto snap = ws.generate_snapshot();
    // candidate and handling are alerts; closed is not
    EXPECT_EQ(snap.snapshot_state.health_alerts.size(), 2u);
}

TEST(WorldStateTest, SnapshotVersionIncrement) {
    WorldState ws;
    Person p; p.person_id = "p1";
    ws.add_person(p);          // version 1
    ws.set_home_mode(HomeMode::DAYTIME);  // version 2
    ws.set_medication_urgency(true);      // version 3

    auto snap = ws.generate_snapshot();
    EXPECT_EQ(snap.snapshot_version, 3);
}

TEST(WorldStateTest, SnapshotRuntimeState) {
    WorldState ws;
    BatteryState bs; bs.level_percent = 42.0f; bs.charging = true;
    ws.set_battery_state(bs);
    NetworkState ns; ns.status = "weak";
    ws.set_network_state(ns);
    ws.set_home_mode(HomeMode::NIGHTTIME);
    ws.set_medication_urgency(true);
    ws.set_manual_service_state("connected");

    auto snap = ws.generate_snapshot();
    EXPECT_FLOAT_EQ(snap.snapshot_state.battery_state.level_percent, 42.0f);
    EXPECT_TRUE(snap.snapshot_state.battery_state.charging);
    EXPECT_EQ(snap.snapshot_state.network_state.status, "weak");
    EXPECT_EQ(snap.snapshot_state.home_mode, HomeMode::NIGHTTIME);
    EXPECT_TRUE(snap.snapshot_state.medication_urgency);
    EXPECT_EQ(snap.snapshot_state.manual_service_state, "connected");
}

// ===== Version =====

TEST(WorldStateTest, InitialVersionIsZero) {
    WorldState ws;
    EXPECT_EQ(ws.version(), 0);
}

TEST(WorldStateTest, VersionBumpsOnMutation) {
    WorldState ws;
    EXPECT_EQ(ws.version(), 0);

    Person p; p.person_id = "p1";
    ws.add_person(p);
    EXPECT_EQ(ws.version(), 1);

    ws.remove_person("p1");
    EXPECT_EQ(ws.version(), 2);
}

TEST(WorldStateTest, VersionNotBumpedOnFailedRemove) {
    WorldState ws;
    EXPECT_FALSE(ws.remove_person("nonexistent"));
    EXPECT_EQ(ws.version(), 0);
}

// ===== Other entity types basic tests =====

TEST(WorldStateTest, PlaceCrud) {
    WorldState ws;
    Place p; p.place_id = "kitchen"; p.category = "room";
    ws.add_place(p);
    auto* got = ws.get_place("kitchen");
    ASSERT_NE(got, nullptr);
    EXPECT_EQ(got->category, "room");
    EXPECT_TRUE(ws.remove_place("kitchen"));
    EXPECT_EQ(ws.get_place("kitchen"), nullptr);
}

TEST(WorldStateTest, ObjectCrud) {
    WorldState ws;
    Object o; o.object_id = "cup1"; o.category = "cup";
    ws.add_object(o);
    ASSERT_NE(ws.get_object("cup1"), nullptr);
    EXPECT_TRUE(ws.remove_object("cup1"));
}

TEST(WorldStateTest, HealthProfileCrud) {
    WorldState ws;
    HealthProfile hp; hp.health_profile_id = "hp1"; hp.person_id = "p1";
    ws.add_health_profile(hp);
    auto* got = ws.get_health_profile("hp1");
    ASSERT_NE(got, nullptr);
    EXPECT_EQ(got->person_id, "p1");
    EXPECT_TRUE(ws.remove_health_profile("hp1"));
}

TEST(WorldStateTest, MedicationAssetCrud) {
    WorldState ws;
    MedicationAsset ma; ma.medication_id = "med1"; ma.name = "Aspirin";
    ws.add_medication_asset(ma);
    auto* got = ws.get_medication_asset("med1");
    ASSERT_NE(got, nullptr);
    EXPECT_EQ(got->name, "Aspirin");
    EXPECT_TRUE(ws.remove_medication_asset("med1"));
}

TEST(WorldStateTest, RoleBindingCrud) {
    WorldState ws;
    RoleBinding rb; rb.binding_id = "rb1"; rb.role = Role::ELDER;
    ws.add_role_binding(rb);
    auto* got = ws.get_role_binding("rb1");
    ASSERT_NE(got, nullptr);
    EXPECT_EQ(got->role, Role::ELDER);
    EXPECT_TRUE(ws.remove_role_binding("rb1"));
}

TEST(WorldStateTest, HouseholdSetAndGet) {
    WorldState ws;
    Household h;
    h.household_id = "h1";
    h.member_ids = {"p1", "p2"};
    h.home_mode = HomeMode::DAYTIME;
    ws.set_household(h);

    const auto& got = ws.get_household();
    EXPECT_EQ(got.household_id, "h1");
    EXPECT_EQ(got.member_ids.size(), 2u);
    EXPECT_EQ(got.home_mode, HomeMode::DAYTIME);
}
