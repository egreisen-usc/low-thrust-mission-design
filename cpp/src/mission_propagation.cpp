#include <iostream>
#include <iomanip>
#include <fstream>
#include <memory>
#include <cmath>
#include "mission_propagation.h"
#include "orbital_elements.h"

PropagationResult propagateMission(
    const MissionConfig& config,
    double r_departure,
    double r_arrival,
    bool save_trajectory,
    const std::string& output_filename) {
    
    PropagationResult result;
    
    // Initialize orbital state
    double v_circ = std::sqrt(MU_SUN / r_departure);
    MissionState state(r_departure, 0, 0, 0, v_circ, 0,
                      config.spacecraft.initial_mass_kg, 0);
    
    // Create integrator
    std::unique_ptr<Propagator> integrator;
    if (config.integrator == "rk4") {
        integrator = std::make_unique<RK4Propagator>();
    } else {
        integrator = std::make_unique<EulerPropagator>();
    }
    
    // Open output file if requested
    std::ofstream outfile;
    if (save_trajectory && !output_filename.empty()) {
        outfile.open(output_filename);
        outfile << "time(s),x(km),y(km),vx(km/s),vy(km/s),r(km),v(km/s),m(kg),"
        << "ra(km),rp(km),e,a(km)\n";
    }
    
    // Propagation loop
    int step = 0;
    double total_delta_v = 0;
    int coast_step = -1;
    
    while (state.t < config.max_flight_time_s) {
        // Compute orbital elements
        OrbitalElements elements = computeOrbitalElements(state.r, state.v, MU_SUN);
        
        // Store state in history
        result.trajectory_history.push_back(state);
        
        // Write to file if enabled
        if (outfile.is_open()) {
            outfile << std::scientific << std::setprecision(6)
                    << state.t << ","
                    << state.r[0] << ","   // x
                    << state.r[1] << ","   // y
                    << state.v[0] << ","   // vx
                    << state.v[1] << ","   // vy
                    << state.radius() << ","
                    << state.speed() << ","
                    << std::fixed << std::setprecision(2)
                    << state.m << ","
                    << std::scientific << std::setprecision(3)
                    << elements.r_a << ","
                    << elements.r_p << ","
                    << std::fixed << std::setprecision(6)
                    << elements.e << ","
                    << std::scientific << std::setprecision(3)
                    << elements.a << "\n";
        }
        
        // Check coast condition
        if (elements.r_a >= config.coast_threshold * r_arrival && coast_step < 0) {
            coast_step = step;
        }
        
        // Stop if coast reached
        if (coast_step >= 0) {
            result.final_state = state;
            result.total_delta_v = total_delta_v;
            result.coast_step = coast_step;
            break;
        }
        
        // Check fuel
        if (state.m < 100) {
            result.final_state = state;
            result.total_delta_v = total_delta_v;
            result.coast_step = coast_step;
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
    
    if (outfile.is_open()) {
        outfile.close();
    }
    
    return result;
}
