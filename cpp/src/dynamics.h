#ifndef DYNAMICS_H
#define DYNAMICS_H

#include "propagator.h"

// ===========================================================================
// DYNAMICS MODULE: ACCELERATION CALCULATIONS
// ===========================================================================
// This module computes the accelerations acting on the spacecraft:
// 1. Gravitational acceleration from the Sun
// 2. Thrust acceleration from the propulsion system
// 3. Total acceleration (gravity + thrust combined)
//
// All accelerations are in km/s² (not m/s²!)
// ===========================================================================

/// Compute gravitational acceleration due to the Sun
///
/// Physics: The gravitational force on the spacecraft is F = -G*M*m/r²
/// where G is gravitational constant, M is Sun's mass, m is spacecraft mass.
/// The acceleration (force per unit mass) is: a = -μ * r / |r|³
/// where μ = G*M is the Sun's gravitational parameter.
///
/// The negative sign indicates acceleration points toward the Sun (negative
/// direction of position vector).
///
/// Example: At Earth's orbit (r = 1.496e8 km), the gravity acceleration is:
///   a = -μ * r / |r|³ = -(1.327e11) * (1.496e8) / (1.496e8)³
///     = -(1.327e11) / (1.496e8)²
///     ≈ -0.00593 km/s² (pointing toward Sun)
///
/// @param r: position vector (km) [rx, ry, rz]
/// @param mu: gravitational parameter of central body (km³/s²)
///            For Sun: mu = 1.327e11 km³/s²
/// @param a: [output] acceleration vector (km/s²) [ax, ay, az]
///           Computed in-place; direction is toward Sun
void computeGravityAccel(const double r[3], double mu, double a[3]);

/// Compute thrust acceleration from the propulsion system
///
/// Physics: Thrust force is F = thrust_mN (converted to SI).
/// The thrust acceleration is: a = F / m
/// where m is current spacecraft mass.
/// Direction: parallel to velocity (parallel-to-velocity steering control law)
///
/// Unit conversion:
///   - Input thrust: millinewtons (mN)
///   - 1 mN = 1×10⁻³ N = 1×10⁻³ kg⋅m/s²
///   - 1 km = 1000 m, so 1 kg⋅m/s² = 1×10⁻³ kg⋅km/s²
///   - Therefore: 1 mN = 1×10⁻³ × 1×10⁻³ kg⋅km/s² = 1×10⁻⁶ kg⋅km/s²
///   - Acceleration: a = (thrust_mN × 1×10⁻⁶ kg⋅km/s²) / m_kg
///                     = (thrust_mN × 1×10⁻⁶) / m km/s²
///
/// Example: For 1000 mN thrust on 10000 kg spacecraft:
///   a_magnitude = (1000 × 1×10⁻⁶) / 10000 = 1×10⁻⁷ km/s²
///
/// Direction: Along velocity vector (not perpendicular to radius)
///   a_thrust = a_magnitude * (v / |v|)
///   This means thrust always accelerates in the direction of current motion,
///   spiraling outward from starting orbit.
///
/// Special cases:
/// - If thrust = 0 or mass ≈ 0: returns [0, 0, 0]
/// - If velocity ≈ 0: returns [0, 0, 0] (cannot determine thrust direction)
///
/// @param v: velocity vector (km/s) [vx, vy, vz]
/// @param m: current spacecraft mass (kg)
/// @param thrust_mN: thrust magnitude (millinewtons)
/// @param a: [output] acceleration vector (km/s²) [ax, ay, az]
///           Computed in-place; direction is along velocity
void computeThrustAccel(const double v[3], double m, double thrust_mN, double a[3]);

/// Compute total acceleration (gravity + thrust combined)
///
/// Physics: Newton's second law allows acceleration components to add.
/// Total acceleration is the vector sum of gravity and thrust accelerations.
///
/// a_total = a_gravity + a_thrust
///
/// This total acceleration is what gets integrated by RK4/Euler to update
/// the spacecraft's velocity and position.
///
/// Example: At Earth orbit with High-Power Hall thruster:
///   a_gravity ≈ -0.00593 km/s² (toward Sun)
///   a_thrust  ≈  0.0000001 km/s² (along velocity)
///   a_total   ≈ -0.00593 km/s² (gravity dominates; thrust is tiny)
///
/// @param state: current spacecraft state (position, velocity, mass, time)
/// @param thrust_mN: thrust magnitude (millinewtons)
/// @param mu: gravitational parameter (km³/s²)
/// @param a: [output] acceleration vector (km/s²) [ax, ay, az]
///           Computed in-place; combination of both effects
void computeAcceleration(const MissionState& state, double thrust_mN, 
                        double mu, double a[3]);

#endif // DYNAMICS_H
