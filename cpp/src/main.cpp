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
    
    // Initialize orbital state
    double r_dep = getOrbitalRadius(config.departure_body);
    double r_arr = getOrbitalRadius(config.arrival_body);
    double v_circ = std::sqrt(MU_SUN / r_dep);
    
    MissionState state(r_dep, 0, 0, 0, v_circ, 0, 
                       config.spacecraft.initial_mass_kg, 0);
    
    std::cout << "Initial State:\n";
    std::cout << "  Position: " << std::scientific << std::setprecision(3) 
              << state.radius() << " km\n";
    std::cout << "  Velocity: " << state.speed() << " km/s\n";
    std::cout << "  Mass: " << std::fixed << std::setprecision(1) 
              << state.m << " kg\n\n";
    
    // Create integrator
    std::unique_ptr<Propagator> integrator;
    if (config.integrator == "rk4") {
        integrator = std::make_unique<RK4Propagator>();
        std::cout << "Using RK4 integrator\n\n";
    } else {
        integrator = std::make_unique<EulerPropagator>();
        std::cout << "Using Euler integrator\n\n";
    }
    
    // Create output file
    std::string results_dir = "../results";
    createDirectory(results_dir);
    std::ofstream outfile(results_dir + "/" + config.output_filename);
    outfile << "time(s),r(km),v(km/s),m(kg),ra(km),rp(km),e,a(km)\n";
    
    // Propagation loop
    int step = 0;
    double total_delta_v = 0;
    int coast_step = -1;
    
    std::cout << "Propagating...\n";
    
    while (state.t < config.max_flight_time_s) {
        // Compute orbital elements
        OrbitalElements elements = computeOrbitalElements(state.r, state.v, MU_SUN);
        
        // Write to file
        outfile << std::scientific << std::setprecision(6)
                << state.t << ","
                << state.r << ","
                << state.v << ","
                << std::fixed << std::setprecision(2)
                << state.m << ","
                << std::scientific << std::setprecision(3)
                << elements.r_a << ","
                << elements.r_p << ","
                << std::fixed << std::setprecision(6)
                << elements.e << ","
                << std::scientific << std::setprecision(3)
                << elements.a << "\n";
        
        // Check coast condition
        if (elements.r_a >= config.coast_threshold * r_arr && coast_step < 0) {
            coast_step = step;
            std::cout << "  Coast activated at step " << step 
                      << " (t=" << std::fixed << std::setprecision(1) 
                      << state.t / 86400.0 << " days)\n";
            std::cout << "    Apoapsis: " << std::scientific << std::setprecision(3) 
                      << elements.r_a << " km\n";
            std::cout << "    Target apoapsis: " << config.coast_threshold * r_arr << " km\n";
            // BREAK immediately upon coast activation
            break;
        }
        
        // Check fuel
        if (state.m < 100) {
            std::cout << "  Fuel depleted at step " << step << "\n";
            break;
        }

        
        // Calculate delta-V for this step
        if (config.spacecraft.thrust_mN > 1e-10) {
            double thrust_accel = (config.spacecraft.thrust_mN * 1e-6) / state.m;
            double delta_v_step = thrust_accel * config.timestep_s;
            total_delta_v += delta_v_step;
        }
        
        // Integration step
        integrator->step(state, config.timestep_s,
                        config.spacecraft.thrust_mN, config.spacecraft.isp_s,
                        MU_SUN, G0);
        
        step++;
    }
    
    outfile.close();
    
    std::cout << "\nPropagation Complete!\n\n";
        std::cout << "Final State:\n";
    std::cout << "  Time: " << std::fixed << std::setprecision(2) 
              << state.t / 86400.0 << " days\n";
    std::cout << "  Position: " << std::scientific << std::setprecision(3) 
              << state.radius() << " km\n";
    std::cout << "  Velocity: " << std::setprecision(3) << state.speed() << " km/s\n";
    std::cout << "  Mass: " << std::fixed << std::setprecision(1) 
              << state.m << " kg\n";
    std::cout << "  Total Delta-V: " << std::scientific << std::setprecision(3) 
              << total_delta_v << " km/s\n";
    std::cout << "  Fuel Consumed: " << std::fixed << std::setprecision(1) 
              << (config.spacecraft.initial_mass_kg - state.m) << " kg\n\n";
    
    OrbitalElements final_elements = computeOrbitalElements(state.r, state.v, MU_SUN);
    std::cout << "Final Orbital Elements:\n";
    std::cout << "  Apoapsis: " << std::scientific << std::setprecision(3) 
              << final_elements.r_a << " km\n";
    std::cout << "  Periapsis: " << final_elements.r_p << " km\n";
    std::cout << "  Eccentricity: " << std::fixed << std::setprecision(6) 
              << final_elements.e << "\n";
    std::cout << "  Semi-major axis: " << std::scientific << std::setprecision(3) 
              << final_elements.a << " km\n\n";
    
    std::cout << "Results saved to: results/" << config.output_filename << "\n";
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
