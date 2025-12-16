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
    // Display a reference table of all celestial bodies and their orbital radii
    
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
    // Create four representative thruster configurations:
    // - Low-Power Hall: moderate thrust, lower ISP
    // - High-Power Hall: high thrust, moderate ISP
    // - Low-Power Ion: low thrust, high ISP
    // - High-Power Ion: moderate thrust, very high ISP
    //
    // These represent the trade-offs between thrust (faster transfer) and
    // ISP (fuel efficiency).
    
    std::cout << "Available Spacecraft:\n";
    std::cout << "---------------------\n";
    
    SpacecraftConfig hall_low, hall_high, ion_low, ion_high;
    
    // Configuration 1: Low-Power Hall Thruster
    // Trade-off: Low thrust (slow spiral) but moderate fuel efficiency
    hall_low.name = "Low-Power Hall";
    hall_low.thrust_mN = 60;        // 60 mN: weak but efficient
    hall_low.isp_s = 1500;          // ISP: 1500 s
    hall_low.initial_mass_kg = 10000;
    
    // Configuration 2: High-Power Hall Thruster
    // Trade-off: High thrust (fast spiral) and good fuel efficiency
    hall_high.name = "High-Power Hall";
    hall_high.thrust_mN = 1000;     // 1000 mN: strong for a low-thrust system
    hall_high.isp_s = 2750;         // ISP: 2750 s (best of the Hall thrusters)
    hall_high.initial_mass_kg = 10000;
    
    // Configuration 3: Low-Power Ion Thruster
    // Trade-off: Moderate thrust, excellent fuel efficiency
    ion_low.name = "Low-Power Ion";
    ion_low.thrust_mN = 250;        // 250 mN: mid-range
    ion_low.isp_s = 4000;           // ISP: 4000 s (very efficient)
    ion_low.initial_mass_kg = 10000;
    
    // Configuration 4: High-Power Ion Thruster
    // Trade-off: Moderate thrust, outstanding fuel efficiency
    ion_high.name = "High-Power Ion";
    ion_high.thrust_mN = 450;       // 450 mN: decent thrust for ion system
    ion_high.isp_s = 9000;          // ISP: 9000 s (extremely efficient)
    ion_high.initial_mass_kg = 10000;
    
    SpacecraftConfig spacecrafts[] = {hall_low, hall_high, ion_low, ion_high};
    
    for (int i = 0; i < 4; i++) {
        const auto& sc = spacecrafts[i];
        double v_e = sc.isp_s * G0;  // Exhaust velocity: v_e = ISP * g0
        printf("  [%d] %-20s | Thrust: %6.0f mN | ISP: %5.0f s | Exhaust Vel: %6.2f km/s\n",
               i, sc.name.c_str(), sc.thrust_mN, sc.isp_s, v_e);
    }
    
    std::cout << "\n";
    
    // ===========================================================================
    // SECTION 3: CREATE EXAMPLE MISSION
    // ===========================================================================
    // Set up a specific mission scenario: Earth to Mars using High-Power Hall
    
    std::cout << "Example Mission Configuration:\n";
    std::cout << "------------------------------\n";
    
    MissionConfig config;
    config.departure_body = parseBodyName("Earth");
    config.arrival_body = parseBodyName("Mars");
    config.spacecraft = hall_high;  // Select High-Power Hall thruster
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
    // Set up the spacecraft in circular orbit at Earth's position
    // Velocity computed for circular orbit: v_circ = sqrt(μ / r)
    
    std::cout << "Initialized Mission State:\n";
    std::cout << "--------------------------\n";
    
    double r_dep = getOrbitalRadius(config.departure_body);  // Earth orbit radius
    double r_arr = getOrbitalRadius(config.arrival_body);    // Mars orbit radius
    double v_circ = std::sqrt(MU_SUN / r_dep);               // Circular orbit velocity
    
    // Create spacecraft state: position at departure, circular velocity, initial mass
    MissionState state(r_dep, 0, 0,       // Position: (r_Earth, 0, 0)
                       0, v_circ, 0,      // Velocity: (0, v_circ, 0) - circular orbit
                       config.spacecraft.initial_mass_kg, 0);  // Mass and time
    
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
    // SECTION 5: DYNAMICS TEST - VERIFY GRAVITY CALCULATION
    // ===========================================================================
    // Verify that gravity acceleration matches theoretical circular orbit
    // For circular orbit: a_centripetal = v² / r (from kinematics)
    //                   a_gravity = -μ / r² (from gravity)
    // These should be equal in magnitude.
    
    std::cout << "Dynamics Test: Gravity Acceleration\n";
    std::cout << "-----------------------------------\n";
    
    double a[3];
    
    // Compute gravity acceleration at current position
    computeGravityAccel(state.r, MU_SUN, a);
    double a_grav_mag = std::sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    
    // Compute expected centripetal acceleration for circular orbit
    // a_c = v² / r
    double a_centripetal = (v_circ * v_circ) / r_dep;
    
    // Compute relative error (should be < 1e-10, essentially machine epsilon)
    double error_grav = std::abs(a_grav_mag - a_centripetal) / a_centripetal * 100;
    
    std::cout << "  Gravity acceleration:       " << std::scientific << a_grav_mag 
              << " km/s²\n";
    std::cout << "  Expected (v²/r):            " << a_centripetal << " km/s²\n";
    std::cout << "  Relative error:             " << std::fixed << std::setprecision(2) 
              << error_grav << " %\n";
    std::cout << "  Status:                     ";
    if (error_grav < 0.01) {
        std::cout << "✓ PASS (gravity is correctly balanced)\n";
    } else {
        std::cout << "✗ FAIL\n";
    }
    std::cout << "\n";
    
    // ===========================================================================
    // SECTION 6: DYNAMICS TEST - VERIFY THRUST CALCULATION
    // ===========================================================================
    // Verify that thrust acceleration matches F = m*a
    // Expected: a = (thrust_mN * 1e-6 kg⋅km/s²) / m_kg
    
    std::cout << "Dynamics Test: Thrust Acceleration\n";
    std::cout << "----------------------------------\n";
    
    // Compute thrust acceleration at current state
    computeThrustAccel(state.v, state.m, config.spacecraft.thrust_mN, a);
    double a_thrust_mag = std::sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    
    // Expected thrust acceleration magnitude
    double a_thrust_expected = (config.spacecraft.thrust_mN * 1e-6) / state.m;
    
    // Verify direction: should be parallel to velocity
    // Compute angle between thrust acceleration and velocity
    double dot_product = a[0]*state.v[0] + a[1]*state.v[1] + a[2]*state.v[2];
    double v_mag = state.speed();
    double cos_angle = dot_product / (a_thrust_mag * v_mag);
    
    std::cout << "  Thrust acceleration:        " << std::scientific << a_thrust_mag 
              << " km/s²\n";
    std::cout << "  Expected (F/m):             " << a_thrust_expected << " km/s²\n";
    std::cout << "  Direction angle to velocity:" << std::fixed << std::setprecision(1)
              << std::acos(cos_angle) * 180 / 3.14159 << " degrees\n";
    std::cout << "  Status:                     ";
    if (std::abs(a_thrust_mag - a_thrust_expected) / a_thrust_expected < 0.01 &&
        std::abs(cos_angle) > 0.9999) {
        std::cout << "✓ PASS (thrust is correct magnitude and direction)\n";
    } else {
        std::cout << "✗ FAIL\n";
    }
    std::cout << "\n";
    
    // ===========================================================================
    // SECTION 7: DYNAMICS TEST - TOTAL ACCELERATION
    // ===========================================================================
    // Verify that total acceleration is the vector sum of gravity and thrust
    
    std::cout << "Dynamics Test: Total Acceleration\n";
    std::cout << "---------------------------------\n";
    
    // Compute total acceleration
    computeAcceleration(state, config.spacecraft.thrust_mN, MU_SUN, a);
    double a_total_mag = std::sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    
    std::cout << "  Total acceleration:         " << std::scientific << a_total_mag 
              << " km/s²\n";
    std::cout << "  Gravity dominates:          " << (a_grav_mag / a_total_mag) 
              << " (ratio)\n";
    std::cout << "  Thrust contribution:        " << (a_thrust_mag / a_total_mag)
              << " (ratio)\n";
    std::cout << "  Components:                 a = (" << a[0] << ", " << a[1] 
              << ", " << a[2] << ") km/s²\n";
    std::cout << "\n";
    
    return 0;
}
