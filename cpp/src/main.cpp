#include <iostream>
#include <iomanip>
#include <cmath>
#include "constants.h"
#include "propagator.h"
#include "dynamics.h"

int main() {
    // ===========================================================================
    // HEADER
    // ===========================================================================
    
    std::cout << "Low-Thrust Mission Design Propagator\n";
    std::cout << "======================================\n\n";
    
    // ===========================================================================
    // SECTION 1: SHOW ALL AVAILABLE BODIES
    // ===========================================================================
    
    std::cout << "Available Celestial Bodies:\n";
    std::cout << "---------------------------\n";
    
    CelestialBody bodies[] = {
        CelestialBody::MERCURY,
        CelestialBody::VENUS,
        CelestialBody::EARTH,
        CelestialBody::MARS,
        CelestialBody::JUPITER,
        CelestialBody::SATURN,
        CelestialBody::URANUS,
        CelestialBody::NEPTUNE,
        CelestialBody::PLUTO
    };
    
    for (const auto& body : bodies) {
        double radius = getOrbitalRadius(body);
        const char* name = getBodyName(body);
        printf("  %-10s : %12.3e km\n", name, radius);
    }
    
    std::cout << "\n";
    
    // ===========================================================================
    // SECTION 2: DEFINE SAMPLE SPACECRAFT
    // ===========================================================================
    
    std::cout << "Available Spacecraft:\n";
    std::cout << "---------------------\n";
    
    SpacecraftConfig hall_low, hall_high, ion_low, ion_high;
    
    hall_low.name = "Low-Power Hall";
    hall_low.thrust_mN = 60;
    hall_low.isp_s = 1500;
    hall_low.initial_mass_kg = 10000;
    
    hall_high.name = "High-Power Hall";
    hall_high.thrust_mN = 1000;
    hall_high.isp_s = 2750;
    hall_high.initial_mass_kg = 10000;
    
    ion_low.name = "Low-Power Ion";
    ion_low.thrust_mN = 250;
    ion_low.isp_s = 4000;
    ion_low.initial_mass_kg = 10000;
    
    ion_high.name = "High-Power Ion";
    ion_high.thrust_mN = 450;
    ion_high.isp_s = 9000;
    ion_high.initial_mass_kg = 10000;
    
    SpacecraftConfig spacecrafts[] = {hall_low, hall_high, ion_low, ion_high};
    
    for (int i = 0; i < 4; i++) {
        const auto& sc = spacecrafts[i];
        double v_e = sc.isp_s * G0;
        printf("  [%d] %-20s | Thrust: %6.0f mN | ISP: %5.0f s | Exhaust Vel: %6.2f km/s\n",
               i, sc.name.c_str(), sc.thrust_mN, sc.isp_s, v_e);
    }
    
    std::cout << "\n";
    
    // ===========================================================================
    // SECTION 3: CREATE EXAMPLE MISSION
    // ===========================================================================
    
    std::cout << "Example Mission Configuration:\n";
    std::cout << "------------------------------\n";
    
    MissionConfig config;
    config.departure_body = parseBodyName("Earth");
    config.arrival_body = parseBodyName("Mars");
    config.spacecraft = hall_high;
    config.timestep_s = 10000;
    
    std::cout << "Mission:\n";
    std::cout << "  Departure: " << getBodyName(config.departure_body) << "\n";
    std::cout << "  Arrival:   " << getBodyName(config.arrival_body) << "\n";
    std::cout << "\n";
    
    std::cout << "Spacecraft:\n";
    std::cout << "  Name:       " << config.spacecraft.name << "\n";
    std::cout << "  Thrust:     " << config.spacecraft.thrust_mN << " mN\n";
    std::cout << "  ISP:        " << config.spacecraft.isp_s << " s\n";
    std::cout << "  Total Mass: " << config.spacecraft.initial_mass_kg << " kg\n";
    std::cout << "\n";
    
    // ===========================================================================
    // SECTION 4: INITIALIZE MISSION STATE
    // ===========================================================================
    
    std::cout << "Initialized Mission State:\n";
    std::cout << "--------------------------\n";
    
    double r_dep = getOrbitalRadius(config.departure_body);
    double r_arr = getOrbitalRadius(config.arrival_body);
    double v_circ = std::sqrt(MU_SUN / r_dep);
    
    MissionState state(r_dep, 0, 0,
                       0, v_circ, 0,
                       config.spacecraft.initial_mass_kg, 0);
    
    std::cout << "  Departure body radius:    " << std::scientific << r_dep << " km\n";
    std::cout << "  Arrival body radius:      " << r_arr << " km\n";
    std::cout << "  Initial position (km):    " << state.radius() << "\n";
    std::cout << "  Initial velocity (km/s):  " << std::fixed << std::setprecision(2) 
              << state.speed() << "\n";
    std::cout << "  Initial mass (kg):        " << std::setprecision(0) << state.m << "\n";
    std::cout << "  Distance to travel (km):  " << std::scientific 
              << (r_arr - r_dep) << "\n";
    std::cout << "\n";
    
    // ===========================================================================
    // SECTION 5: TEST TIME INTEGRATORS
    // ===========================================================================
    // Verify that RK4 and Euler can take steps correctly
    
    std::cout << "Time Integrator Test:\n";
    std::cout << "---------------------\n";
    
    // Create two copies of state for comparison
    MissionState state_rk4 = state;
    MissionState state_euler = state;
    
    // Create integrators
    RK4Propagator rk4;
    EulerPropagator euler;
    
    // Take one step with each method
    double dt = config.timestep_s;
    
    rk4.step(state_rk4, dt, config.spacecraft.thrust_mN, config.spacecraft.isp_s, 
             MU_SUN, G0);
    
    euler.step(state_euler, dt, config.spacecraft.thrust_mN, config.spacecraft.isp_s, 
               MU_SUN, G0);
    
    // Display results
    std::cout << "After one timestep (dt = " << dt << " s):\n\n";
    
    std::cout << "RK4 Integrator:\n";
    std::cout << "  Position (km):    " << std::scientific << state_rk4.radius() << "\n";
    std::cout << "  Velocity (km/s):  " << std::fixed << std::setprecision(4) 
              << state_rk4.speed() << "\n";
    std::cout << "  Mass (kg):        " << std::setprecision(1) << state_rk4.m << "\n";
    std::cout << "  Time (s):         " << std::scientific << state_rk4.t << "\n";
    std::cout << "\n";
    
    std::cout << "Euler Integrator:\n";
    std::cout << "  Position (km):    " << std::scientific << state_euler.radius() << "\n";
    std::cout << "  Velocity (km/s):  " << std::fixed << std::setprecision(4) 
              << state_euler.speed() << "\n";
    std::cout << "  Mass (kg):        " << std::setprecision(1) << state_euler.m << "\n";
    std::cout << "  Time (s):         " << std::scientific << state_euler.t << "\n";
    std::cout << "\n";
    
    // Compare methods
    double pos_diff = std::abs(state_rk4.radius() - state_euler.radius());
    double vel_diff = std::abs(state_rk4.speed() - state_euler.speed());
    
    std::cout << "Comparison (RK4 vs Euler):\n";
    std::cout << "  Position difference:  " << std::scientific << pos_diff << " km\n";
    std::cout << "  Velocity difference:  " << std::fixed << std::setprecision(6) 
              << vel_diff << " km/s\n";
    std::cout << "  Status:               ";
    
    if (pos_diff < 1e5 && vel_diff < 0.01) {
        std::cout << "✓ PASS (both integrators working)\n";
    } else {
        std::cout << "✗ FAIL\n";
    }
    
    std::cout << "\n";
    std::cout << "Ready to propagate full trajectories!\n";
    
    return 0;
}
