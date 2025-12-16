#ifndef PROPAGATOR_H
#define PROPAGATOR_H

#include <cmath>
#include <string>
#include "constants.h"

// ===========================================================================
// MISSION STATE STRUCT
// ===========================================================================

struct MissionState {
    double r[3];  // Position (km)
    double v[3];  // Velocity (km/s)
    double m;     // Mass (kg)
    double t;     // Time (s)
    
    MissionState() : m(0), t(0) {
        r[0] = r[1] = r[2] = 0;
        v[0] = v[1] = v[2] = 0;
    }
    
    MissionState(double x, double y, double z,
                 double vx, double vy, double vz,
                 double mass, double time = 0.0)
        : m(mass), t(time) {
        r[0] = x;   r[1] = y;   r[2] = z;
        v[0] = vx;  v[1] = vy;  v[2] = vz;
    }
    
    double radius() const {
        return std::sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    }
    
    double speed() const {
        return std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    }
};

// ===========================================================================
// SPACECRAFT CONFIGURATION STRUCT
// ===========================================================================

struct SpacecraftConfig {
    std::string name = "Default Spacecraft";
    double thrust_mN = 1000;             // millinewtons
    double isp_s = 2750;                 // specific impulse (seconds)
    double initial_mass_kg = 10000;      // total spacecraft mass (kg)
};

// ===========================================================================
// MISSION CONFIGURATION STRUCT
// ===========================================================================

struct MissionConfig {
    // Mission targets
    CelestialBody departure_body = CelestialBody::EARTH;
    CelestialBody arrival_body = CelestialBody::MARS;
    
    // Spacecraft to use
    SpacecraftConfig spacecraft;
    
    // Integration parameters
    std::string integrator = "rk4";      // "rk4" or "euler"
    double timestep_s = 10000;           // seconds
    
    // Termination condition
    double max_flight_time_s = 7.884e8;  // ~25 years
    double coast_threshold = 0.999;      // coast when apoapsis >= threshold * target_radius
    
    // Output file
    std::string output_filename = "results/trajectory.csv";
};

// ===========================================================================
// PROPAGATOR BASE CLASS
// ===========================================================================

class Propagator {
public:
    virtual ~Propagator() = default;
    
    /// Advance state by one timestep
    /// @param state: [in/out] spacecraft state
    /// @param dt: timestep (s)
    /// @param thrust_mN: thrust magnitude (mN)
    /// @param isp_s: specific impulse (s)
    /// @param mu: gravitational parameter (km³/s²)
    /// @param g0: gravitational acceleration constant (km/s²)
    virtual void step(MissionState& state, double dt,
                     double thrust_mN, double isp_s,
                     double mu, double g0) = 0;
};

// ===========================================================================
// CONCRETE PROPAGATORS
// ===========================================================================

/// 4th-order Runge-Kutta integrator
class RK4Propagator : public Propagator {
public:
    void step(MissionState& state, double dt,
             double thrust_mN, double isp_s,
             double mu, double g0) override;
};

/// Forward Euler integrator
class EulerPropagator : public Propagator {
public:
    void step(MissionState& state, double dt,
             double thrust_mN, double isp_s,
             double mu, double g0) override;
};

#endif // PROPAGATOR_H
