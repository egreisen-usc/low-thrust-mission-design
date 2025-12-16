#include <cmath>
#include "propagator.h"
#include "dynamics.h"

// ===========================================================================
// RK4 PROPAGATOR IMPLEMENTATION
// ===========================================================================
// 4th-order Runge-Kutta time integrator
// Local truncation error: O(dt⁵)
// Global error: O(dt⁴)
//
// The RK4 method evaluates the acceleration at 4 points within each timestep
// and takes a weighted average to advance the state. This gives much higher
// accuracy than simple Euler method with comparable computational cost.
//
// Reference: Numerical Recipes in C, Press et al.

void RK4Propagator::step(MissionState& state, double dt,
                         double thrust_mN, double isp_s,
                         double mu, double g0, int thrust_direction) {
    
    // ===========================================================================
    // STAGE 1: Evaluate at beginning of interval
    // ===========================================================================
    
    // k1 = acceleration at (t, r, v)
    double k1[3];
    computeAcceleration(state, thrust_mN, mu, k1, thrust_direction);
    
    // Velocity at current time (used for position update)
    double v_k1[3] = {state.v[0], state.v[1], state.v[2]};
    
    // ===========================================================================
    // STAGE 2: Evaluate at midpoint (t + dt/2)
    // ===========================================================================
    
    // Create temporary state at midpoint
    // Position: r + v*dt/2
    double r_mid[3] = {
        state.r[0] + state.v[0] * (dt / 2),
        state.r[1] + state.v[1] * (dt / 2),
        state.r[2] + state.v[2] * (dt / 2)
    };
    
    // Velocity: v + k1*dt/2
    double v_mid[3] = {
        state.v[0] + k1[0] * (dt / 2),
        state.v[1] + k1[1] * (dt / 2),
        state.v[2] + k1[2] * (dt / 2)
    };
    
    // Create temporary state for acceleration evaluation
    MissionState state_mid;
    state_mid.r[0] = r_mid[0]; state_mid.r[1] = r_mid[1]; state_mid.r[2] = r_mid[2];
    state_mid.v[0] = v_mid[0]; state_mid.v[1] = v_mid[1]; state_mid.v[2] = v_mid[2];
    state_mid.m = state.m;  // Mass doesn't change during acceleration evaluation
    
    // k2 = acceleration at midpoint
    double k2[3];
    computeAcceleration(state_mid, thrust_mN, mu, k2, thrust_direction);
    
    // Velocity at midpoint
    double v_k2[3] = {v_mid[0], v_mid[1], v_mid[2]};
    
    // ===========================================================================
    // STAGE 3: Evaluate at midpoint again (different velocity)
    // ===========================================================================
    
    // Position: r + v*dt/2 (same as stage 2)
    // Velocity: v + k2*dt/2 (different from stage 2)
    double v_mid2[3] = {
        state.v[0] + k2[0] * (dt / 2),
        state.v[1] + k2[1] * (dt / 2),
        state.v[2] + k2[2] * (dt / 2)
    };
    
    // Create temporary state
    state_mid.v[0] = v_mid2[0]; state_mid.v[1] = v_mid2[1]; state_mid.v[2] = v_mid2[2];
    
    // k3 = acceleration at this midpoint configuration
    double k3[3];
    computeAcceleration(state_mid, thrust_mN, mu, k3, thrust_direction);
    
    // Velocity at this midpoint
    double v_k3[3] = {v_mid2[0], v_mid2[1], v_mid2[2]};
    
    // ===========================================================================
    // STAGE 4: Evaluate at end of interval (t + dt)
    // ===========================================================================
    
    // Position: r + v*dt + k3*dt²/2
    double r_end[3] = {
        state.r[0] + state.v[0] * dt + k3[0] * (dt * dt / 2),
        state.r[1] + state.v[1] * dt + k3[1] * (dt * dt / 2),
        state.r[2] + state.v[2] * dt + k3[2] * (dt * dt / 2)
    };
    
    // Velocity: v + k3*dt
    double v_end[3] = {
        state.v[0] + k3[0] * dt,
        state.v[1] + k3[1] * dt,
        state.v[2] + k3[2] * dt
    };
    
    // Create temporary state at end of interval
    MissionState state_end;
    state_end.r[0] = r_end[0]; state_end.r[1] = r_end[1]; state_end.r[2] = r_end[2];
    state_end.v[0] = v_end[0]; state_end.v[1] = v_end[1]; state_end.v[2] = v_end[2];
    state_end.m = state.m;
    
    // k4 = acceleration at end of interval
    double k4[3];
    computeAcceleration(state_end, thrust_mN, mu, k4, thrust_direction);
    
    // Velocity at end of interval
    double v_k4[3] = {v_end[0], v_end[1], v_end[2]};
    
    // ===========================================================================
    // COMBINE STAGES: Weighted average of 4 estimates
    // ===========================================================================
    // RK4 formula: y(t+dt) = y(t) + (dt/6) * (k1 + 2*k2 + 2*k3 + k4)
    // Weights: [1, 2, 2, 1] / 6 = [1/6, 1/3, 1/3, 1/6]
    
    // Update velocity
    state.v[0] = state.v[0] + (dt / 6.0) * (k1[0] + 2*k2[0] + 2*k3[0] + k4[0]);
    state.v[1] = state.v[1] + (dt / 6.0) * (k1[1] + 2*k2[1] + 2*k3[1] + k4[1]);
    state.v[2] = state.v[2] + (dt / 6.0) * (k1[2] + 2*k2[2] + 2*k3[2] + k4[2]);
    
    // Update position using weighted average of velocities at 4 stages
    // This is more accurate than just using initial velocity
    state.r[0] = state.r[0] + (dt / 6.0) * (v_k1[0] + 2*v_k2[0] + 2*v_k3[0] + v_k4[0]);
    state.r[1] = state.r[1] + (dt / 6.0) * (v_k1[1] + 2*v_k2[1] + 2*v_k3[1] + v_k4[1]);
    state.r[2] = state.r[2] + (dt / 6.0) * (v_k1[2] + 2*v_k2[2] + 2*v_k3[2] + v_k4[2]);
    
    // Update time
    state.t = state.t + dt;
    
    // Update mass (due to propellant consumption)
    // dm/dt = -thrust_mN / (isp_s * g0 * 1e6)
    // The 1e6 converts from mN to kg*km/s²
    if (thrust_mN > 1e-10 && isp_s > 1e-10) {
        double v_e = isp_s * g0;  // Exhaust velocity (km/s)
        double dm_dt = -thrust_mN * 1e-6 / v_e;  // Mass flow rate (kg/s)
        state.m = state.m + dm_dt * dt;
        
        // Prevent negative mass
        if (state.m < 0) {
            state.m = 0;
        }
    }
}

// ===========================================================================
// EULER PROPAGATOR IMPLEMENTATION
// ===========================================================================
// Forward Euler time integrator (1st order)
// Local truncation error: O(dt²)
// Global error: O(dt)
//
// The Euler method is the simplest time integrator: just evaluate
// acceleration at the current point and step forward.
//
// Advantages: Simple, fast, easy to understand
// Disadvantages: Low accuracy, requires small timesteps for good results
//
// Reference: Numerical Recipes in C, Press et al.

void EulerPropagator::step(MissionState& state, double dt,
                           double thrust_mN, double isp_s,
                           double mu, double g0, int thrust_direction) {
    
    // ===========================================================================
    // STEP 1: Evaluate acceleration at current state
    // ===========================================================================
    
    double a[3];
    computeAcceleration(state, thrust_mN, mu, a, thrust_direction);
    
    // ===========================================================================
    // STEP 2: Update velocity
    // ===========================================================================
    // v(t+dt) = v(t) + a(t) * dt
    
    state.v[0] = state.v[0] + a[0] * dt;
    state.v[1] = state.v[1] + a[1] * dt;
    state.v[2] = state.v[2] + a[2] * dt;
    
    // ===========================================================================
    // STEP 3: Update position
    // ===========================================================================
    // r(t+dt) = r(t) + v(t) * dt
    // (uses velocity at beginning of step, not updated velocity)
    
    state.r[0] = state.r[0] + state.v[0] * dt;
    state.r[1] = state.r[1] + state.v[1] * dt;
    state.r[2] = state.r[2] + state.v[2] * dt;
    
    // ===========================================================================
    // STEP 4: Update time
    // ===========================================================================
    
    state.t = state.t + dt;
    
    // ===========================================================================
    // STEP 5: Update mass (due to propellant consumption)
    // ===========================================================================
    // dm/dt = -F / v_e = -(thrust_mN * 1e-6) / (isp_s * g0)
    
    if (thrust_mN > 1e-10 && isp_s > 1e-10) {
        double v_e = isp_s * g0;  // Exhaust velocity (km/s)
        double dm_dt = -thrust_mN * 1e-6 / v_e;  // Mass flow rate (kg/s)
        state.m = state.m + dm_dt * dt;
        
        // Prevent negative mass
        if (state.m < 0) {
            state.m = 0;
        }
    }
}
