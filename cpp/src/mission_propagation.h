#ifndef MISSION_PROPAGATION_H
#define MISSION_PROPAGATION_H

#include <string>
#include <vector>
#include "propagator.h"
#include "constants.h"
#include "orbital_elements.h"

// ===========================================================================
// PROPAGATION RESULT STRUCTURE
// ===========================================================================
// Contains both trajectory history and final mission result

struct PropagationResult {
    MissionState final_state;
    double total_delta_v;
    int coast_step;
    std::vector<MissionState> trajectory_history;
    
    PropagationResult() : total_delta_v(0), coast_step(-1) {}
};

// ===========================================================================
// SHARED PROPAGATION ENGINE
// ===========================================================================

/// Propagates a mission from start to coast or fuel depletion
/// Returns full trajectory history and final state
PropagationResult propagateMission(
    const MissionConfig& config,
    double r_departure,
    double r_arrival,
    bool save_trajectory = false,
    const std::string& output_filename = ""
);

#endif // MISSION_PROPAGATION_H
