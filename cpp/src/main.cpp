#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <vector>
#include <sys/stat.h>
#include <errno.h>
#include <yaml-cpp/yaml.h>
#include "constants.h"
#include "propagator.h"
#include "dynamics.h"
#include "orbital_elements.h"
#include "comparison.h"
#include "mission_batch.h"
#include "mission_propagation.h"

// Forward declarations - these are defined in mission_batch.cpp
bool createDirectory(const std::string& path);
MissionConfig loadConfigFromYAML(const std::string& filename);

// ===========================================================================
// MISSION RUNNER (Single Mission)
// ===========================================================================

void runSingleMissionMode(const std::string& config_path) {
    std::cout << "\n";
    std::cout << "=====================================================\n";
    std::cout << "ORBITAL TRANSFER PROPAGATOR - SINGLE MISSION MODE\n";
    std::cout << "=====================================================\n\n";
    
    // Load configuration
    MissionConfig config = loadConfigFromYAML(config_path);
    
    std::cout << "Configuration loaded from: " << config_path << "\n";
    std::cout << "  Spacecraft: " << config.spacecraft.name << "\n";
    std::cout << "  Thrust: " << config.spacecraft.thrust_mN << " mN\n";
    std::cout << "  ISP: " << config.spacecraft.isp_s << " s\n";
    std::cout << "  Initial Mass: " << config.spacecraft.initial_mass_kg << " kg\n";
    std::cout << "  From: " << getBodyName(config.departure_body) << "\n";
    std::cout << "  To: " << getBodyName(config.arrival_body) << "\n";
    std::cout << "  Integrator: " << config.integrator << "\n";
    std::cout << "  Timestep: " << config.timestep_s << " s\n\n";
    
    // Get orbital radii
    double r_dep = getOrbitalRadius(config.departure_body);
    double r_arr = getOrbitalRadius(config.arrival_body);
    
    std::cout << "Initial State:\n";
    std::cout << "  Position: " << std::scientific << std::setprecision(3) 
              << r_dep << " km\n";
    double v_circ = std::sqrt(MU_SUN / r_dep);
    std::cout << "  Velocity: " << v_circ << " km/s\n";
    std::cout << "  Mass: " << std::fixed << std::setprecision(1) 
              << config.spacecraft.initial_mass_kg << " kg\n\n";
    
    std::cout << "Using " << (config.integrator == "rk4" ? "RK4" : "Euler") << " integrator\n\n";
    
    // Create results directory
    std::string results_dir = "../results";
    createDirectory(results_dir);
    
    // Propagate mission
    std::cout << "Propagating...\n";
    PropagationResult prop_result = propagateMission(config, r_dep, r_arr, true,
                                                     results_dir + "/" + config.output_filename);
    
    if (prop_result.coast_step >= 0) {
        std::cout << "  Coast activated at step " << prop_result.coast_step 
                  << " (t=" << std::fixed << std::setprecision(1) 
                  << prop_result.final_state.t / 86400.0 << " days)\n";
    }
    
    std::cout << "\nPropagation Complete!\n\n";
    std::cout << "Final State:\n";
    std::cout << "  Time: " << std::fixed << std::setprecision(2) 
              << prop_result.final_state.t / 86400.0 << " days\n";
    std::cout << "  Position: " << std::scientific << std::setprecision(3) 
              << prop_result.final_state.radius() << " km\n";
    std::cout << "  Velocity: " << std::setprecision(3) 
              << prop_result.final_state.speed() << " km/s\n";
    std::cout << "  Mass: " << std::fixed << std::setprecision(1) 
              << prop_result.final_state.m << " kg\n";
    std::cout << "  Total Delta-V: " << std::scientific << std::setprecision(3) 
              << prop_result.total_delta_v << " km/s\n";
    std::cout << "  Fuel Consumed: " << std::fixed << std::setprecision(1) 
              << (config.spacecraft.initial_mass_kg - prop_result.final_state.m) << " kg\n\n";
    
    OrbitalElements final_elements = computeOrbitalElements(prop_result.final_state.r,
                                                            prop_result.final_state.v, MU_SUN);
    std::cout << "Final Orbital Elements:\n";
    std::cout << "  Apoapsis: " << std::scientific << std::setprecision(3) 
              << final_elements.r_a << " km\n";
    std::cout << "  Periapsis: " << final_elements.r_p << " km\n";
    std::cout << "  Eccentricity: " << std::fixed << std::setprecision(6) 
              << final_elements.e << "\n";
    std::cout << "  Semi-major axis: " << std::scientific << std::setprecision(3) 
              << final_elements.a << " km\n\n";
    
    std::cout << "Results saved to: " << results_dir << "/" << config.output_filename << "\n";
    std::cout << "=====================================================\n\n";
}


// ===========================================================================
// BATCH MISSION RUNNER
// ===========================================================================

void runBatchMissionMode(const std::string& batch_config_file) {
    std::cout << "\n";
    std::cout << "=====================================================\n";
    std::cout << "ORBITAL TRANSFER PROPAGATOR - BATCH MODE\n";
    std::cout << "=====================================================\n\n";
    
    // Read batch configuration file
    std::vector<std::string> config_files;
    std::ifstream batch_file(batch_config_file);
    
    if (!batch_file.is_open()) {
        std::cerr << "Error: Cannot open batch config file: " << batch_config_file << "\n";
        return;
    }
    
    std::string line;
    while (std::getline(batch_file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        // Trim whitespace
        size_t start = line.find_first_not_of(" \t");
        size_t end = line.find_last_not_of(" \t");
        if (start != std::string::npos) {
            config_files.push_back(line.substr(start, end - start + 1));
        }
    }
    batch_file.close();
    
    std::cout << "Batch configuration loaded: " << batch_config_file << "\n";
    std::cout << "Missions to run: " << config_files.size() << "\n\n";
    
    // Create batch runner and execute missions
    MissionBatchRunner batch_runner;
    MissionComparison comparison = batch_runner.runBatchMissions(config_files);
    
    // Print summary to console
    std::cout << "\n";
    comparison.printSummary();
    
    // Write detailed comparison CSV
    std::string results_dir = "../results";
    createDirectory(results_dir);
    comparison.writeComparisonCSV(results_dir + "/mission_comparison.csv");


    
    std::cout << "=====================================================\n\n";
}

// ===========================================================================
// MAIN ENTRY POINT
// ===========================================================================

int main(int argc, char* argv[]) {
    std::cout << "\n";
    std::cout << "╔═════════════════════════════════════════════════════╗\n";
    std::cout << "║  ORBITAL TRANSFER PROPAGATOR WITH ELECTRIC THRUST  ║\n";
    std::cout << "║                    Version 1.0                      ║\n";
    std::cout << "╚═════════════════════════════════════════════════════╝\n";
    
    // Check command line arguments
    if (argc == 1) {
        // Default: run single mission
        std::cout << "\nUsage:\n";
        std::cout << "  Single mission:  ./propagator <config.yaml>\n";
        std::cout << "  Batch missions:  ./propagator --batch <batch_config.txt>\n\n";
        
        std::cout << "No arguments provided. Running default single mission mode...\n";
        runSingleMissionMode("../config/earth_mars_baseline.yaml");
        
    } else if (argc == 2 && std::string(argv[1]) == "--batch") {
        std::cerr << "Error: --batch flag requires a config file argument\n";
        std::cerr << "Usage: ./propagator --batch <batch_config.txt>\n";
        return 1;
        
    } else if (argc >= 2 && std::string(argv[1]) == "--batch") {
        // Batch mode
        std::string batch_config = argv[2];
        runBatchMissionMode(batch_config);
        
    } else if (argc == 2) {
        // Single mission mode with specified config
        runSingleMissionMode(argv[1]);
        
    } else {
        std::cerr << "Error: Invalid arguments\n";
        std::cerr << "Usage:\n";
        std::cerr << "  Single mission:  ./propagator <config.yaml>\n";
        std::cerr << "  Batch missions:  ./propagator --batch <batch_config.txt>\n";
        return 1;
    }
    
    return 0;
}
