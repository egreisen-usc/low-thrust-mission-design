#ifndef MISSION_BATCH_H
#define MISSION_BATCH_H

#include <string>
#include <vector>
#include "comparison.h"

// ===========================================================================
// BATCH MISSION RUNNER
// ===========================================================================
// Runs multiple missions and collects results for comparison

class MissionBatchRunner {
public:
    /// Run a single mission from config file and return result
    MissionResult runSingleMission(const std::string& config_file);
    
    /// Run all missions in configuration vector
    MissionComparison runBatchMissions(const std::vector<std::string>& config_files);
    
private:
    /// Helper function to run main propagation logic
    /// Returns a MissionResult with all metrics
    MissionResult propagateMission(const std::string& config_path,
                                  const std::string& mission_name);
};

#endif // MISSION_BATCH_H
