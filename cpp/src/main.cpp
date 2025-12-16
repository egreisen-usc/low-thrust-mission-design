#include <iostream>
#include <iomanip>
#include "constants.h"
#include "propagator.h"

int main() {
    std::cout << "Low-Thrust Mission Design Propagator\n";
    std::cout << "======================================\n\n";
    
    // ===========================================================================
    // SHOW ALL AVAILABLE BODIES
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
    // DEFINE SAMPLE SPACECRAFT
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
    // CREATE EXAMPLE MISSION
    // ===========================================================================
    
    std::cout << "Example Mission Configuration:\n";
    std::cout << "------------------------------\n";
    
    MissionConfig config;
    config.departure_body = parseBodyName("Earth");
    config.arrival_body = parseBodyName("Mars");
    config.spacecraft = hall_high;  // Select High-Power Hall
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
    // INITIALIZE MISSION STATE
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
    
    return 0;
}
