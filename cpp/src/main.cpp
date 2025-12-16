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

// ===========================================================================
// CONFIGURATION LOADER
// ===========================================================================

MissionConfig loadConfigFromYAML(const std::string& filename) {
    MissionConfig config;
    
    try {
        YAML::Node yaml = YAML::LoadFile(filename);
        
        // Load mission section
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
        
        // Load spacecraft section
        if (yaml["spacecraft"]) {
            YAML::Node spacecraft = yaml["spacecraft"];
            
            if (spacecraft["name"]) {
                std::string name = spacecraft["name"].as<std::string>();
                config.spacecraft.name = name;
                
                // Set thruster parameters based on name
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
        
        // Load integration section
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
        
        // Load propagation section
        if (yaml["propagation"]) {
            YAML::Node propagation = yaml["propagation"];
            
            if (propagation["coast_threshold"]) {
                config.coast_threshold = propagation["coast_threshold"].as<double>();
            }
        }
        
        // Load output section
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
// TRAJECTORY OUTPUT TO CSV
// ===========================================================================

void writeCSVHeader(std::ofstream& file) {
    file << "Time(s),Position_X(km),Position_Y(km),Position_Z(km),"
         << "Velocity_X(km/s),Velocity_Y(km/s),Velocity_Z(km/s),"
         << "Mass(kg),Radius(km),Speed(km/s),Apoapsis(km),Periapsis(km),"
         << "Eccentricity,SemiMajorAxis(km)\n";
}

void writeCSVRow(std::ofstream& file, const MissionState& state, 
                 const OrbitalElements& elements) {
    file << std::scientific << std::setprecision(6)
         << state.t << ","
         << state.r[0] << "," << state.r[1] << "," << state.r[2] << ","
         << state.v[0] << "," << state.v[1] << "," << state.v[2] << ","
         << state.m << ","
         << state.radius() << "," << state.speed() << ","
         << elements.r_a << "," << elements.r_p << ","
         << elements.e << "," << elements.a << "\n";
}

// ===========================================================================
// MAIN PROPAGATION LOOP
// ===========================================================================

int main() {
    std::cout << "Low-Thrust Mission Design Propagator\n";
    std::cout << "======================================\n\n";
    
    // Load configuration
    std::cout << "Loading configuration file...\n";
    std::string config_path = "../config/earth_mars_baseline.yaml";
    MissionConfig config = loadConfigFromYAML(config_path);
    
    std::cout << "  Mission: " << getBodyName(config.departure_body) << " → "
              << getBodyName(config.arrival_body) << "\n";
    std::cout << "  Spacecraft: " << config.spacecraft.name << "\n";
    std::cout << "  Integrator: " << config.integrator << "\n";
    std::cout << "  Timestep: " << config.timestep_s << " s\n";
    std::cout << "\n";
    
    // Initialize state
    double r_dep = getOrbitalRadius(config.departure_body);
    double r_arr = getOrbitalRadius(config.arrival_body);
    double v_circ = std::sqrt(MU_SUN / r_dep);
    
    MissionState state(r_dep, 0, 0,
                       0, v_circ, 0,
                       config.spacecraft.initial_mass_kg, 0);
    
    std::cout << "Initial State:\n";
    std::cout << "  Position: " << std::scientific << state.radius() << " km\n";
    std::cout << "  Velocity: " << std::fixed << std::setprecision(2) 
              << state.speed() << " km/s\n";
    std::cout << "  Mass: " << std::setprecision(0) << state.m << " kg\n";
    std::cout << "\n";
    
    // Create integrator
    std::unique_ptr<Propagator> integrator;
    if (config.integrator == "rk4") {
        integrator = std::make_unique<RK4Propagator>();
        std::cout << "Using RK4 (4th-order Runge-Kutta) integrator\n";
    } else {
        integrator = std::make_unique<EulerPropagator>();
        std::cout << "Using Euler (1st-order) integrator\n";
    }
    
    std::cout << "\n";
    
    // Create output directory
    std::string output_dir = "../results";
    if (createDirectory(output_dir)) {
        std::cout << "Output directory ready: " << output_dir << "\n";
    } else {
        std::cerr << "Warning: Could not create output directory\n";
    }
    std::cout << "\n";
    
    // Open output file
    std::string output_path = "../" + config.output_filename;
    std::ofstream outfile(output_path);
    if (!outfile.is_open()) {
        std::cerr << "Error: Cannot open output file: " << output_path << "\n";
        return 1;
    }
    
    writeCSVHeader(outfile);
    
    // Propagation loop
    std::cout << "Propagating trajectory...\n";
    std::cout << std::string(80, '-') << "\n";
    std::cout << std::left << std::setw(10) << "Step"
              << std::setw(15) << "Time (days)"
              << std::setw(15) << "Radius (km)"
              << std::setw(15) << "Velocity (km/s)"
              << std::setw(15) << "Mass (kg)"
              << std::setw(10) << "Status\n";
    std::cout << std::string(80, '-') << "\n";
    
    int step = 0;
    double total_delta_v = 0;
    
    while (state.t < config.max_flight_time_s) {
        // Compute orbital elements
        OrbitalElements elements = computeOrbitalElements(state.r, state.v, MU_SUN);
        
        // Write to file
        writeCSVRow(outfile, state, elements);
        
        // Print status
        if (step % 1000 == 0) {
            double time_days = state.t / 86400.0;
            std::cout << std::left << std::setw(10) << step
                      << std::setw(15) << std::fixed << std::setprecision(1) << time_days
                      << std::setw(15) << std::scientific << std::setprecision(2) 
                      << state.radius()
                      << std::setw(15) << std::fixed << std::setprecision(2) 
                      << state.speed()
                      << std::setw(15) << std::setprecision(0) << state.m
                      << std::setw(10) << "THRUST" << "\n";
        }
        
        // Check coast condition - END SIMULATION when coast is reached
        if (elements.r_a >= config.coast_threshold * r_arr) {
            std::cout << "\n*** MISSION COAST CONDITION REACHED ***\n";
            std::cout << "*** Total integration steps: " << step << "\n";
            std::cout << "*** Thrust phase duration: " << std::fixed << state.t / 86400.0 << " days ***\n";
            std::cout << "*** Final apoapsis: " << std::scientific << elements.r_a << " km ***\n\n";
            break;
        }
        
        // Check fuel condition
        if (state.m < 100) {
            std::cout << "\n*** MISSION TERMINATED ***\n";
            std::cout << "*** Out of fuel (mass = " << state.m << " kg) ***\n\n";
            break;
        }
        
        // Calculate delta-V for this step before integration
        // Δv = (thrust / mass) × dt
        if (config.spacecraft.thrust_mN > 1e-10) {
            double thrust_accel = (config.spacecraft.thrust_mN * 1e-6) / state.m;  // km/s²
            double delta_v_step = thrust_accel * config.timestep_s;  // km/s
            total_delta_v += delta_v_step;
        }
        
        // Take integration step (always thrust - no coast phase)
        integrator->step(state, config.timestep_s,
                        config.spacecraft.thrust_mN, config.spacecraft.isp_s,
                        MU_SUN, G0);
        
        step++;
    }
    
    outfile.close();
    
    // Final summary
    std::cout << std::string(80, '-') << "\n";
    
    OrbitalElements final_elements = computeOrbitalElements(state.r, state.v, MU_SUN);
    
    std::cout << "\nMission Summary:\n";
    std::cout << "  Thrust phase duration: " << std::fixed << std::setprecision(1) 
              << (state.t / 86400.0) << " days ("
              << (state.t / 86400.0 / 365.25) << " years)\n";
    std::cout << "  Total delta-V: " << std::fixed << std::setprecision(2) 
              << total_delta_v << " km/s\n";
    std::cout << "  Final position: " << std::scientific << state.radius() << " km\n";
    std::cout << "  Final velocity: " << std::fixed << std::setprecision(2) 
              << state.speed() << " km/s\n";
    std::cout << "  Mass remaining: " << std::setprecision(0) << state.m << " kg\n";
    std::cout << "  Propellant consumed: " 
              << (config.spacecraft.initial_mass_kg - state.m) << " kg\n";
    std::cout << "\nOrbital Elements (at coast activation):\n";
    std::cout << "  Semi-major axis: " << std::scientific << final_elements.a << " km\n";
    std::cout << "  Eccentricity: " << std::fixed << std::setprecision(6) 
              << final_elements.e << "\n";
    std::cout << "  Periapsis: " << std::scientific << final_elements.r_p << " km\n";
    std::cout << "  Apoapsis: " << final_elements.r_a << " km\n";
    std::cout << "  Inclination: " << std::fixed << std::setprecision(2) 
              << (final_elements.i * 180 / 3.14159265358979323846) << " degrees\n";
    
    std::cout << "\nResults saved to: " << output_path << "\n";
    
    return 0;
}
