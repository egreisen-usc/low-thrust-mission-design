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

// ===========================================================================
// DIRECTORY CREATION HELPER (from main.cpp)
// ===========================================================================

bool createDirectory(const std::string& path) {
    #ifdef _WIN32
        return _mkdir(path.c_str()) == 0;
    #else
        return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
    #endif
}

// ===========================================================================
// CONFIGURATION LOADER (from main.cpp)
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
    
    // Initialize state
    double r_dep = getOrbitalRadius(config.departure_body);
    double r_arr = getOrbitalRadius(config.arrival_body);
    double v_circ = std::sqrt(MU_SUN / r_dep);
    
    MissionState state(r_dep, 0, 0, 0, v_circ, 0, 
                       config.spacecraft.initial_mass_kg, 0);
    
    // Create integrator
    std::unique_ptr<Propagator> integrator;
    if (config.integrator == "rk4") {
        integrator = std::make_unique<RK4Propagator>();
    } else {
        integrator = std::make_unique<EulerPropagator>();
    }
    
    // Propagation loop
    int step = 0;
    double total_delta_v = 0;
    
    while (state.t < config.max_flight_time_s) {
        OrbitalElements elements = computeOrbitalElements(state.r, state.v, MU_SUN);
        
        // Check coast condition
        if (elements.r_a >= config.coast_threshold * r_arr) {
            result.flight_time_days = state.t / 86400.0;
            result.total_delta_v_km_s = total_delta_v;
            result.final_mass_kg = state.m;
            result.propellant_consumed_kg = config.spacecraft.initial_mass_kg - state.m;
            result.final_apoapsis_km = elements.r_a;
            result.final_periapsis_km = elements.r_p;
            result.final_eccentricity = elements.e;
            result.final_semi_major_axis_km = elements.a;
            break;
        }
        
        // Check fuel
        if (state.m < 100) {
            result.flight_time_days = state.t / 86400.0;
            result.total_delta_v_km_s = total_delta_v;
            result.final_mass_kg = state.m;
            result.propellant_consumed_kg = config.spacecraft.initial_mass_kg - state.m;
            result.final_apoapsis_km = 0;
            break;
        }
        
        // Calculate delta-V
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
