#include <cmath>
#include "dynamics.h"

// ===========================================================================
// GRAVITATIONAL ACCELERATION IMPLEMENTATION
// ===========================================================================

void computeGravityAccel(const double r[3], double mu, double a[3]) {
    // Step 1: Compute magnitude of position vector |r| = sqrt(rx² + ry² + rz²)
    double r_mag = std::sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    
    // Step 2: Safety check - avoid division by zero
    // (In practice, this should never happen because spacecraft starts at
    //  significant distance from Sun)
    if (r_mag < 1e-10) {
        a[0] = a[1] = a[2] = 0;
        return;
    }
    
    // Step 3: Compute |r|³ (used in the a = -μ * r / |r|³ formula)
    double r_cubed = r_mag * r_mag * r_mag;
    
    // Step 4: Compute common factor: -μ / |r|³
    // This factor is applied to each component of r
    double factor = -mu / r_cubed;
    
    // Step 5: Compute acceleration components: a = factor * r
    // The negative sign makes acceleration point toward Sun (negative r direction)
    a[0] = factor * r[0];
    a[1] = factor * r[1];
    a[2] = factor * r[2];
    
    // Result: a = -μ / |r|³ * [rx, ry, rz]
    //        = -μ * r / |r|³ (vector form)
}

// ===========================================================================
// THRUST ACCELERATION IMPLEMENTATION
// ===========================================================================

void computeThrustAccel(const double v[3], double m, double thrust_mN, 
                        double a[3], int thrust_direction) {
    // Step 1: Check for degenerate cases
    // If thrust is negligible or mass is zero, no acceleration
    if (thrust_mN < 1e-10 || m < 1e-10) {
        a[0] = a[1] = a[2] = 0;
        return;
    }
    
    // Step 2: Compute magnitude of velocity vector |v| = sqrt(vx² + vy² + vz²)
    double v_mag = std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    
    // Step 3: Check for degenerate case
    // If velocity is nearly zero, cannot determine thrust direction
    // (In practice, spacecraft always has some orbital velocity)
    if (v_mag < 1e-10) {
        a[0] = a[1] = a[2] = 0;
        return;
    }
    
    // Step 4: Compute thrust acceleration magnitude
    // F = thrust_mN * 1e-6 kg⋅km/s² (unit conversion)
    // a_magnitude = F / m = (thrust_mN * 1e-6) / m km/s²
    //
    // Derivation of unit conversion:
    //   1 mN = 1e-3 N (prefix: milli)
    //   1 N = 1 kg⋅m/s²
    //   So: 1 mN = 1e-3 kg⋅m/s²
    //   In km/s²: 1 m = 1e-3 km
    //   1e-3 kg⋅m/s² = 1e-3 kg * (1e-3 km) / s² = 1e-6 kg⋅km/s²
    //
    // Therefore: a = (thrust_mN * 1e-6 kg⋅km/s²) / m_kg
    double a_mag = (thrust_mN * 1e-6) / m;
    
    // Step 5: Determine thrust direction: parallel or antiparallel to velocity
    // thrust_direction: +1 for prograde (accelerate), -1 for retrograde (decelerate)
    // Unit vector in velocity direction: v_unit = v / |v|
    // Thrust acceleration: a_thrust = thrust_direction * a_mag * v_unit
    //                               = thrust_direction * a_mag * v / |v|
    // Factor to apply to each velocity component: thrust_direction * a_mag / |v|
    double factor = thrust_direction * a_mag / v_mag;
    
    // Step 6: Compute acceleration components: a = factor * v
    // This gives a vector parallel (or antiparallel) to velocity with magnitude a_mag
    a[0] = factor * v[0];
    a[1] = factor * v[1];
    a[2] = factor * v[2];
    
    // Result: a = thrust_direction * (thrust_mN * 1e-6 / m) * (v / |v|) km/s²
    //        Direction: +1 along velocity (prograde), -1 opposite (retrograde)
    //        Magnitude: (thrust_mN * 1e-6) / m km/s²
}


// ===========================================================================
// TOTAL ACCELERATION IMPLEMENTATION
// ===========================================================================

void computeAcceleration(const MissionState& state, double thrust_mN, 
                        double mu, double a[3], int thrust_direction) {
    // Step 1: Compute gravitational acceleration
    // This includes both the magnitude and direction of gravity
    double a_grav[3];
    computeGravityAccel(state.r, mu, a_grav);
    
    // Step 2: Compute thrust acceleration
    // This is zero during coast phase (thrust_mN = 0)
    // thrust_direction: +1 for prograde (outward), -1 for retrograde (inward)
    double a_thrust[3];
    computeThrustAccel(state.v, state.m, thrust_mN, a_thrust, thrust_direction);
    
    // Step 3: Combine into total acceleration
    // Newton's second law allows acceleration components to be added vectorially
    // a_total = a_gravity + a_thrust
    a[0] = a_grav[0] + a_thrust[0];
    a[1] = a_grav[1] + a_thrust[1];
    a[2] = a_grav[2] + a_thrust[2];
    
    // Result: a = a_gravity + a_thrust (vector sum)
    //        This is what RK4/Euler integrates to update velocity/position
}
