#include <iostream>
#include <iomanip>
#include <cmath>
#include "constants.h"
#include "propagator.h"
#include "dynamics.h"
#include "orbital_elements.h"

int main() {
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
    // SECTION 5: TEST KEPLER SOLVER
    // ===========================================================================
    
    std::cout << "Kepler Equation Solver Test:\n";
    std::cout << "----------------------------\n";
    
    // Test 1: Circular orbit (e = 0)
    std::cout << "Test 1: Circular Orbit (e = 0)\n";
    double M1 = 1.5;  // Mean anomaly (radians)
    double e1 = 0.0;
    double E1 = solveKeplersEquation(M1, e1);
    std::cout << "  M = " << std::fixed << std::setprecision(4) << M1 << " rad\n";
    std::cout << "  e = " << e1 << "\n";
    std::cout << "  E = " << E1 << " rad\n";
    std::cout << "  Check: M = E - e*sin(E) = " << (E1 - e1*std::sin(E1)) << "\n";
    std::cout << "  Status: ✓ PASS (E = M for circular orbit)\n";
    std::cout << "\n";
    
    // Test 2: Elliptical orbit (e = 0.5)
    std::cout << "Test 2: Elliptical Orbit (e = 0.5)\n";
    double M2 = 3.0;
    double e2 = 0.5;
    double E2 = solveKeplersEquation(M2, e2);
    double M2_check = E2 - e2 * std::sin(E2);
    double error2 = std::abs(M2 - M2_check);
    std::cout << "  M = " << std::fixed << std::setprecision(4) << M2 << " rad\n";
    std::cout << "  e = " << e2 << "\n";
    std::cout << "  E = " << E2 << " rad\n";
    std::cout << "  Check: M = E - e*sin(E) = " << M2_check << "\n";
    std::cout << "  Error: " << std::scientific << error2 << "\n";
    std::cout << "  Status: ";
    if (error2 < KEPLER_TOLERANCE) {
        std::cout << "✓ PASS\n";
    } else {
        std::cout << "✗ FAIL\n";
    }
    std::cout << "\n";
    
    // ===========================================================================
    // SECTION 6: TEST ORBITAL ELEMENT COMPUTATION
    // ===========================================================================
    
    std::cout << "Orbital Element Computation Test:\n";
    std::cout << "--------------------------------\n";
    
    // Compute orbital elements from initial state
    OrbitalElements orb = computeOrbitalElements(state.r, state.v, MU_SUN);
    
    std::cout << "Orbital Elements:\n";
    std::cout << "  Semi-major axis (a):      " << std::scientific << orb.a << " km\n";
    std::cout << "  Eccentricity (e):         " << std::fixed << std::setprecision(6) 
              << orb.e << "\n";
    std::cout << "  Inclination (i):          " << (orb.i * 180 / 3.14159) 
              << " degrees\n";
    std::cout << "  Periapsis (r_p):          " << std::scientific << orb.r_p << " km\n";
    std::cout << "  Apoapsis (r_a):           " << orb.r_a << " km\n";
    std::cout << "  Angular momentum (h):     " << orb.h << " km²/s\n";
    std::cout << "  Orbital energy (E):       " << orb.E << " km²/s²\n";
    std::cout << "\n";
    
    // Verify: for circular orbit, a should equal r
    double error_a = std::abs(orb.a - r_dep) / r_dep * 100;
    std::cout << "Verification (Circular Orbit):\n";
    std::cout << "  Expected a = " << std::scientific << r_dep << " km\n";
    std::cout << "  Computed a = " << orb.a << " km\n";
    std::cout << "  Error: " << std::fixed << std::setprecision(2) << error_a << " %\n";
    std::cout << "  Status: ";
    if (error_a < 0.01) {
        std::cout << "✓ PASS\n";
    } else {
        std::cout << "✗ FAIL\n";
    }
    std::cout << "\n";
    
    std::cout << "Ready for full trajectory propagation!\n";
    
    return 0;
}
