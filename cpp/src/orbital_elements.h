#ifndef ORBITAL_ELEMENTS_H
#define ORBITAL_ELEMENTS_H

#include "propagator.h"

// ===========================================================================
// ORBITAL ELEMENTS STRUCTURE
// ===========================================================================
// Classical orbital elements describe an orbit using 6 parameters:
// a (semi-major axis), e (eccentricity), i (inclination),
// Ω (longitude of ascending node), ω (argument of periapsis),
// ν (true anomaly)
//
// These are useful for:
// - Orbit classification and mission planning
// - Event detection (apoapsis/periapsis)
// - Trajectory analysis and diagnostics
// ===========================================================================

struct OrbitalElements {
    // Classical orbital elements
    double a;      // Semi-major axis (km)
    double e;      // Eccentricity (dimensionless, 0 to 1)
    double i;      // Inclination (radians, 0 to π)
    double Omega;  // Longitude of ascending node (radians, 0 to 2π)
    double omega;  // Argument of periapsis (radians, 0 to 2π)
    double nu;     // True anomaly (radians, 0 to 2π)
    
    // Derived quantities (computed from state vectors)
    double r_p;    // Periapsis radius (km) = a(1-e)
    double r_a;    // Apoapsis radius (km) = a(1+e)
    double h;      // Specific orbital angular momentum (km²/s)
    double E;      // Specific orbital energy (km²/s²)
    
    /// Constructor: initialize to zero
    OrbitalElements() : a(0), e(0), i(0), Omega(0), omega(0), nu(0),
                       r_p(0), r_a(0), h(0), E(0) {}
};

// ===========================================================================
// KEPLER EQUATION SOLVER
// ===========================================================================

/// Solve Kepler's equation using Newton-Raphson iteration
///
/// Kepler's equation: M = E - e * sin(E)
/// where M is mean anomaly, E is eccentric anomaly, e is eccentricity.
///
/// This solves for E given M and e using Newton-Raphson method:
/// E_{n+1} = E_n - (E_n - e*sin(E_n) - M) / (1 - e*cos(E_n))
///
/// Convergence:
/// - Quadratic convergence (error squared at each iteration)
/// - Typically converges in 3-5 iterations for e < 0.9
/// - Initial guess: E_0 = M works well for most orbits
///
/// Special cases:
/// - If e ≈ 0 (circular orbit): E ≈ M (only 1 iteration needed)
/// - If e → 1 (parabolic orbit): slow convergence, may need more iterations
/// - If e > 1: not valid (hyperbolic orbit, use different equation)
///
/// @param M: mean anomaly (radians)
/// @param e: eccentricity (0 to 1)
/// @param tolerance: convergence tolerance (default 1e-12)
/// @param max_iterations: maximum iterations (default 20)
/// @return eccentric anomaly E (radians)
double solveKeplersEquation(double M, double e, double tolerance = 1e-12, 
                            int max_iterations = 20);

/// Compute true anomaly from eccentric anomaly
///
/// Relationship: tan(ν/2) = sqrt((1+e)/(1-e)) * tan(E/2)
/// where ν is true anomaly, E is eccentric anomaly, e is eccentricity.
///
/// @param E: eccentric anomaly (radians)
/// @param e: eccentricity
/// @return true anomaly (radians)
double eccentricToTrueAnomaly(double E, double e);

/// Compute orbital elements from state vectors
///
/// Given position r and velocity v (in inertial frame), compute
/// the 6 classical orbital elements.
///
/// Procedure:
/// 1. Compute specific angular momentum: h = r × v
/// 2. Compute eccentricity vector: e_vec = (v × h)/μ - r/|r|
/// 3. Compute semi-major axis: a = -μ / (2*E_specific)
/// 4. Compute inclination: i = arccos(h_z / |h|)
/// 5. Compute longitude of ascending node: Ω = arctan2(h_x, -h_y)
/// 6. Compute argument of periapsis: ω = arctan2(e_z, e_x)
/// 7. Compute true anomaly: ν = arctan2(r·v, h)
/// 8. Compute periapsis/apoapsis: r_p = a(1-e), r_a = a(1+e)
///
/// @param r: position vector (km) [rx, ry, rz]
/// @param v: velocity vector (km/s) [vx, vy, vz]
/// @param mu: gravitational parameter (km³/s²)
/// @return OrbitalElements structure with computed elements
OrbitalElements computeOrbitalElements(const double r[3], const double v[3], 
                                       double mu);

#endif // ORBITAL_ELEMENTS_H
