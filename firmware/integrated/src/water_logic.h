#pragma once
//
// water_logic.h — pure watering decision logic (P2-13 Direction B, DL-159).
//
// The DL-142 dosing policy expressed as free functions of plain values, with NO
// hardware, NO Arduino types, NO globals, NO side effects. fsm.cpp includes this
// and calls these instead of inlining the math, so firmware behavior is identical
// — the point of the extraction is that this logic can be compiled and unit-tested
// on a host (see tests/firmware/), giving the pump-path decision code the automated
// coverage the rest of the FSM (hardware, timers, NVS) can't easily get.
//
// Only <math.h> is used (fabsf / isnan), which exists on-target and on host alike.
//
#include <math.h>

namespace water_logic {

// ---- soil freshness (P0-3) ------------------------------------------------
// Stale if there is no moisture reading yet, or the newest valid reading is older
// than stale_ms. age_ms is (now - last_valid_soil_ms); pass has_reading=false when
// the smoothed moisture is still NaN.
inline bool is_soil_stale(bool has_reading, unsigned long age_ms,
                          unsigned long stale_ms) {
    return (!has_reading) || (age_ms > stale_ms);
}

// ---- trigger (MONITOR) ----------------------------------------------------
// Start an auto cycle only when armed (not in maintenance), the probe is fresh,
// and smoothed moisture has drifted to/below the trigger threshold.
inline bool should_trigger(float moist_pct, bool maintenance, bool soil_stale,
                           float trigger_pct) {
    return (!maintenance) && (!soil_stale) && (moist_pct <= trigger_pct);
}

// ---- target ---------------------------------------------------------------
inline bool target_reached(float moist_pct, float target_pct) {
    return moist_pct >= target_pct;
}

// ---- plateau detection (SETTLE) -------------------------------------------
// After the minimum settle, the probe has "caught up" once it stops changing:
// the reading has moved <= slope_pct over a full plateau window. Returns true
// when the current sample, compared with the armed reference, is within slope_pct
// AND a full window has elapsed since the reference was taken.
inline bool is_plateau(float cur_pct, float ref_pct, unsigned long elapsed_ms,
                       unsigned long window_ms, float slope_pct) {
    if (elapsed_ms < window_ms) return false;
    return fabsf(cur_pct - ref_pct) <= slope_pct;
}

// ---- dose volume clamp (begin_dose) ---------------------------------------
// Clamp a requested dose to the per-dose ceiling and the remaining session
// budget. Returns the mL to actually deliver, or 0 if none may be delivered
// (budget exhausted or request non-positive). Never returns negative.
inline int clamp_dose(int requested_ml, int session_ml, int max_dose_ml,
                      int session_cap_ml) {
    int budget = session_cap_ml - session_ml;
    if (budget <= 0) return 0;
    int ml = requested_ml;
    if (ml > max_dose_ml) ml = max_dose_ml;
    if (ml > budget)      ml = budget;
    if (ml <= 0) return 0;
    return ml;
}

// ---- post-settle evaluate() outcome ---------------------------------------
// The decision after a dose has settled/plateaued, given the smoothed moisture,
// the rise since the dose started, the session total, and whether the one-time
// grace has been used. Mirrors evaluate() exactly.
enum class Decision {
    DONE,        // target reached -> finish
    SUPPLEMENT,  // absorbed but short -> another dose
    STOP_CAPPED, // absorbed but session cap hit -> stop
    GRACE,       // not absorbing, first stall -> one-time grace wait
    STOP_FAILED, // not absorbing, grace already used -> stop
};

inline Decision evaluate_decision(float moist_pct, float rise_pct, int session_ml,
                                  bool grace_used, float target_pct,
                                  float absorb_rise_pct, int session_cap_ml) {
    if (moist_pct >= target_pct) {
        return Decision::DONE;
    }
    if (rise_pct >= absorb_rise_pct) {
        return (session_ml < session_cap_ml) ? Decision::SUPPLEMENT
                                             : Decision::STOP_CAPPED;
    }
    return grace_used ? Decision::STOP_FAILED : Decision::GRACE;
}

}  // namespace water_logic
