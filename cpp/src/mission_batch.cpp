#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <memory>
#include <sys/stat.h>
#include <errno.h>
#include <yaml-cpp/yaml.h>
#include "constants.h"
#include "propagator.h"
#include "dynamics.h"
#include "orbital_elements.h"
#include "mission_batch.h"
#include "mission_propagation.h"

// ===========================================================================
// DIRECTORY CREATION HELPER
// ===========================================================================

bool createDirectory(const std::string& path) {
    #ifdef _WIN32
        return _mkdir(path.c_str()) == 0;
    #else
        return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
    #endif
}

std::string getResultsDirectory() {
    // When running from build/bin/, we need to go up 2 levels to project root
    // Then into results/
    std::string results_path = "../results";
    createDirectory(results_path);
    return results_path;
}

// ===========================================================================
// CONFIGURATION LOADER
// ===========================================================================

MissionConfig loadConfigFromYAML(const std::string& filename) {
    MissionConfig config;
    
    try {
        YAML::Node yaml = YAML::LoadFile(filename);
        
        if (yaml["mission"]) {
            YAML::Node mission = yaml["mission"];
            if (mission["initial_mass_kg"]) {
                config.spacecraft.initial_mass_kg = mission["initial_mass_kg"].as<double>();
            }
            if (mission["departure_body"]) {
                std::string dep = mission["departure_body"].as<std::string>();
                config.departure_body = parseBodyName(dep);
            }
            if (mission["arrival_body"]) {
                std::string arr = mission["arrival_body"].as<std::string>();
                config.arrival_body = parseBodyName(arr);
            }
        }
        
        if (yaml["spacecraft"]) {
            YAML::Node spacecraft = yaml["spacecraft"];
            if (spacecraft["name"]) {
                std::string name = spacecraft["name"].as<std::string>();
                config.spacecraft.name = name;
                
                if (name == "Low-Power Hall") {
                    config.spacecraft.thrust_mN = 60;
                    config.spacecraft.isp_s = 1500;
                } else if (name == "High-Power Hall") {
                    config.spacecraft.thrust_mN = 1000;
                    config.spacecraft.isp_s = 2750;
                } else if (name == "Low-Power Ion") {
                    config.spacecraft.thrust_mN = 250;
                    config.spacecraft.isp_s = 4000;
                } else if (name == "High-Power Ion") {
                    config.spacecraft.thrust_mN = 450;
                    config.spacecraft.isp_s = 9000;
                }
            }
        }
        
        if (yaml["integration"]) {
            YAML::Node integration = yaml["integration"];
            if (integration["method"]) {
                config.integrator = integration["method"].as<std::string>();
            }
            if (integration["timestep_s"]) {
                config.timestep_s = integration["timestep_s"].as<double>();
            }
            if (integration["max_flight_time_s"]) {
                config.max_flight_time_s = integration["max_flight_time_s"].as<double>();
            }
        }
        
        if (yaml["propagation"]) {
            YAML::Node propagation = yaml["propagation"];
            if (propagation["coast_threshold"]) {
                config.coast_threshold = propagation["coast_threshold"].as<double>();
            }
        }
        
        if (yaml["output"]) {
            YAML::Node output = yaml["output"];
            if (output["filename"]) {
                config.output_filename = output["filename"].as<std::string>();
            }
        }
        
    } catch (const YAML::Exception& e) {
        std::cerr << "Error loading config file: " << e.what() << std::endl;
    }
    
    return config;
}

// ===========================================================================
// MISSION BATCH RUNNER IMPLEMENTATION
// ===========================================================================

MissionResult MissionBatchRunner::propagateMission(const std::string& config_path,
                                                   const std::string& mission_name) {
    MissionResult result;
    result.mission_name = mission_name;
    
    // Load configuration
    MissionConfig config = loadConfigFromYAML(config_path);
    
    result.thruster_name = config.spacecraft.name;
    result.departure_body = getBodyName(config.departure_body);
    result.arrival_body = getBodyName(config.arrival_body);
    result.initial_mass_kg = config.spacecraft.initial_mass_kg;
    
    // Get orbital radii
    double r_dep = getOrbitalRadius(config.departure_body);
    double r_arr = getOrbitalRadius(config.arrival_body);
    
    // Create results directory
    std::string results_dir = "../results";
    createDirectory(results_dir);
    
    // Extract base name for trajectory file
    std::string base_name = mission_name;
    size_t last_dot = base_name.find_last_of(".");
    if (last_dot != std::string::npos) {
        base_name = base_name.substr(0, last_dot);
    }
    
    // Propagate mission and save trajectory
    PropagationResult prop_result = ::propagateMission(config, r_dep, r_arr, true,
                                                       results_dir + "/" + base_name + "_trajectory.csv");
    
    // Extract mission results
    result.flight_time_days = prop_result.final_state.t / 86400.0;
    result.total_delta_v_km_s = prop_result.total_delta_v;
    result.final_mass_kg = prop_result.final_state.m;
    result.propellant_consumed_kg = config.spacecraft.initial_mass_kg - prop_result.final_state.m;
    
    // Compute orbital elements at coast
    OrbitalElements elements = computeOrbitalElements(prop_result.final_state.r,
                                                      prop_result.final_state.v, MU_SUN);
    result.final_apoapsis_km = elements.r_a;
    result.final_periapsis_km = elements.r_p;
    result.final_eccentricity = elements.e;
    result.final_semi_major_axis_km = elements.a;
    
    return result;
}

MissionResult MissionBatchRunner::runSingleMission(const std::string& config_file) {
    std::string config_path = "../config/" + config_file;
    return propagateMission(config_path, config_file);
}

MissionComparison MissionBatchRunner::runBatchMissions(const std::vector<std::string>& config_files) {
    MissionComparison comparison;
    
    for (const auto& config_file : config_files) {
        std::cout << "Running mission: " << config_file << "...\n";
        MissionResult result = runSingleMission(config_file);
        comparison.addMission(result);
    }
    
    comparison.computeMetrics();
    return comparison;
}
