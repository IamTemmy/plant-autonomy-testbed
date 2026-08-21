// tests/firmware/test_water_logic.cpp
//
// Host unit tests for firmware/integrated/src/water_logic.h (P2-13 Direction B).
// Pure C++, no Arduino, no gtest — a tiny assertion runner so CI needs only g++.
// Build/run:  g++ -std=c++11 -I../../firmware/integrated/src test_water_logic.cpp -o t && ./t
//
#include <cstdio>
#include <cmath>

#include "water_logic.h"

using namespace water_logic;

static int g_failures = 0;
static int g_checks   = 0;

#define CHECK(cond)                                                          \
    do {                                                                     \
        ++g_checks;                                                          \
        if (!(cond)) {                                                       \
            ++g_failures;                                                    \
            std::printf("  FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);    \
        }                                                                    \
    } while (0)

// ---- is_soil_stale --------------------------------------------------------
static void test_is_soil_stale() {
    // No reading yet -> always stale, regardless of age.
    CHECK(is_soil_stale(false, 0, 30000) == true);
    // Fresh reading, well within window -> not stale.
    CHECK(is_soil_stale(true, 5000, 30000) == false);
    // Exactly at the window -> not yet stale (strict >).
    CHECK(is_soil_stale(true, 30000, 30000) == false);
    // Just past the window -> stale.
    CHECK(is_soil_stale(true, 30001, 30000) == true);
}

// ---- should_trigger -------------------------------------------------------
static void test_should_trigger() {
    // Armed, fresh, below trigger -> yes.
    CHECK(should_trigger(19.0f, /*maint*/false, /*stale*/false, 20.0f) == true);
    // Exactly at trigger (<=) -> yes.
    CHECK(should_trigger(20.0f, false, false, 20.0f) == true);
    // Above trigger -> no.
    CHECK(should_trigger(20.1f, false, false, 20.0f) == false);
    // In maintenance -> never, even if dry.
    CHECK(should_trigger(5.0f, true, false, 20.0f) == false);
    // Stale probe -> never, even if "dry".
    CHECK(should_trigger(5.0f, false, true, 20.0f) == false);
}

// ---- target_reached -------------------------------------------------------
static void test_target_reached() {
    CHECK(target_reached(40.0f, 40.0f) == true);   // inclusive
    CHECK(target_reached(41.0f, 40.0f) == true);
    CHECK(target_reached(39.9f, 40.0f) == false);
}

// ---- is_plateau -----------------------------------------------------------
static void test_is_plateau() {
    // Window not yet elapsed -> not a plateau even if flat.
    CHECK(is_plateau(70.0f, 70.0f, /*elapsed*/500000UL, /*window*/600000UL, 1.0f) == false);
    // Elapsed, flat within slope -> plateau.
    CHECK(is_plateau(70.4f, 70.0f, 600000UL, 600000UL, 1.0f) == true);
    // Elapsed but still climbing beyond slope -> not a plateau.
    CHECK(is_plateau(72.0f, 70.0f, 600000UL, 600000UL, 1.0f) == false);
    // Movement exactly at slope bound -> plateau (<=).
    CHECK(is_plateau(71.0f, 70.0f, 600000UL, 600000UL, 1.0f) == true);
    // Downward drift within slope also counts (abs).
    CHECK(is_plateau(69.2f, 70.0f, 700000UL, 600000UL, 1.0f) == true);
}

// ---- clamp_dose -----------------------------------------------------------
static void test_clamp_dose() {
    // Normal: request within budget and under the per-dose cap.
    CHECK(clamp_dose(/*req*/100, /*session*/0, /*max*/150, /*cap*/600) == 100);
    // Over the per-dose cap -> clamped to cap.
    CHECK(clamp_dose(200, 0, 150, 600) == 150);
    // Near the session cap -> clamped to remaining budget.
    CHECK(clamp_dose(100, 550, 150, 600) == 50);
    // Session cap already reached -> 0 (nothing may flow).
    CHECK(clamp_dose(100, 600, 150, 600) == 0);
    // Over budget AND over cap -> the smaller (budget) wins.
    CHECK(clamp_dose(200, 500, 150, 600) == 100);
    // Non-positive request -> 0.
    CHECK(clamp_dose(0, 0, 150, 600) == 0);
    // Never negative.
    CHECK(clamp_dose(50, 700, 150, 600) == 0);
}

// ---- evaluate_decision ----------------------------------------------------
static void test_evaluate_decision() {
    const float TARGET = 40.0f, ABSORB = 7.0f;
    const int CAP = 600;
    // At/over target -> DONE (regardless of rise).
    CHECK(evaluate_decision(41.0f, 0.0f, 100, false, TARGET, ABSORB, CAP) == Decision::DONE);
    // Absorbed (rise >= absorb) but short of target, budget left -> SUPPLEMENT.
    CHECK(evaluate_decision(30.0f, 8.0f, 100, false, TARGET, ABSORB, CAP) == Decision::SUPPLEMENT);
    // Absorbed but session cap hit -> STOP_CAPPED.
    CHECK(evaluate_decision(30.0f, 8.0f, 600, false, TARGET, ABSORB, CAP) == Decision::STOP_CAPPED);
    // Not absorbing, first stall (grace unused) -> GRACE.
    CHECK(evaluate_decision(22.0f, 2.0f, 100, false, TARGET, ABSORB, CAP) == Decision::GRACE);
    // Not absorbing, grace already used -> STOP_FAILED.
    CHECK(evaluate_decision(22.0f, 2.0f, 100, true, TARGET, ABSORB, CAP) == Decision::STOP_FAILED);
    // Rise exactly at absorb threshold (>=) counts as absorbed.
    CHECK(evaluate_decision(30.0f, 7.0f, 100, false, TARGET, ABSORB, CAP) == Decision::SUPPLEMENT);
    // Target takes precedence even when session is capped.
    CHECK(evaluate_decision(45.0f, 1.0f, 600, true, TARGET, ABSORB, CAP) == Decision::DONE);
}

int main() {
    std::printf("water_logic host tests\n");
    test_is_soil_stale();
    test_should_trigger();
    test_target_reached();
    test_is_plateau();
    test_clamp_dose();
    test_evaluate_decision();
    std::printf("%d checks, %d failure(s)\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
