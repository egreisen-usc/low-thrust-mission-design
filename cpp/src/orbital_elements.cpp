#include <cmath>
#include "orbital_elements.h"

// ===========================================================================
// KEPLER EQUATION SOLVER (NEWTON-RAPHSON)
// ===========================================================================

double solveKeplersEquation(double M, double e, double tolerance, 
                            int max_iterations) {
    // Normalize M to [0, 2π)
    double M_norm = std::fmod(M, 2 * 3.14159265358979323846);
    if (M_norm < 0) M_norm += 2 * 3.14159265358979323846;
    
    // Initial guess: E_0 = M (works well for all eccentricities)
    double E = M_norm;
    
    // Special case: nearly circular orbit
    // For small e, E ≈ M, so just return M
    if (e < 1e-10) {
        return M_norm;
    }
    
    // Newton-Raphson iteration
    for (int iter = 0; iter < max_iterations; iter++) {
        // Compute f(E) = E - e*sin(E) - M
        double sinE = std::sin(E);
        double cosE = std::cos(E);
        double f = E - e * sinE - M_norm;
        
        // Compute f'(E) = 1 - e*cos(E)
        double f_prime = 1.0 - e * cosE;
        
        // Avoid division by zero
        if (std::abs(f_prime) < 1e-15) {
            break;
        }
        
        // Newton-Raphson step: E_{n+1} = E_n - f(E_n) / f'(E_n)
        double E_next = E - f / f_prime;
        
        // Check convergence
        if (std::abs(E_next - E) < tolerance) {
            return E_next;
        }
        
        E = E_next;
    }
    
    // Return best estimate after max iterations
    return E;
}

// ===========================================================================
// ECCENTRIC TO TRUE ANOMALY CONVERSION
// ===========================================================================

double eccentricToTrueAnomaly(double E, double e) {
    // Formula: tan(ν/2) = sqrt((1+e)/(1-e)) * tan(E/2)
    // More stable: ν = 2 * arctan(sqrt((1+e)/(1-e)) * tan(E/2))
    
    // Handle edge cases
    if (e < 0 || e > 1) {
        return 0;  // Invalid eccentricity
    }
    
    if (e < 1e-10) {
        // Circular orbit: ν = E
        return E;
    }
    
    // Compute tan(E/2)
    double half_E = E / 2.0;
    double tan_half_E = std::tan(half_E);
    
    // Compute sqrt((1+e)/(1-e))
    double factor = std::sqrt((1.0 + e) / (1.0 - e));
    
    // Compute tan(ν/2) = factor * tan(E/2)
    double tan_half_nu = factor * tan_half_E;
    
    // Compute ν = 2 * arctan(tan(ν/2))
    double nu = 2.0 * std::atan(tan_half_nu);
    
    // Normalize to [0, 2π)
    if (nu < 0) {
        nu += 2 * 3.14159265358979323846;
    }
    
    return nu;
}

// ===========================================================================
// COMPUTE ORBITAL ELEMENTS FROM STATE VECTORS
// ===========================================================================

OrbitalElements computeOrbitalElements(const double r[3], const double v[3], 
                                       double mu) {
    OrbitalElements elements;
    
    // ===========================================================================
    // STEP 1: Compute specific orbital angular momentum h = r × v
    // ===========================================================================
    
    // Cross product: h = r × v
    double h_vec[3] = {
        r[1]*v[2] - r[2]*v[1],  // h_x = r_y*v_z - r_z*v_y
        r[2]*v[0] - r[0]*v[2],  // h_y = r_z*v_x - r_x*v_z
        r[0]*v[1] - r[1]*v[0]   // h_z = r_x*v_y - r_y*v_x
    };
    
    // Magnitude of h
    double h_mag = std::sqrt(h_vec[0]*h_vec[0] + h_vec[1]*h_vec[1] + h_vec[2]*h_vec[2]);
    elements.h = h_mag;
    
    // ===========================================================================
    // STEP 2: Compute specific orbital energy E = v²/2 - μ/r
    // ===========================================================================
    
    double r_mag = std::sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
    double v_mag_sq = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    
    elements.E = v_mag_sq / 2.0 - mu / r_mag;
    
    // ===========================================================================
    // STEP 3: Compute semi-major axis a = -μ / (2*E)
    // ===========================================================================
    
    if (std::abs(elements.E) > 1e-15) {
        elements.a = -mu / (2.0 * elements.E);
    } else {
        // Parabolic orbit (E = 0): a = infinity
        elements.a = 1e10;
    }
    
    // ===========================================================================
    // STEP 4: Compute eccentricity from h and a
    // ===========================================================================
    // e² = 1 + (2*E*h²/μ²)
    
    if (elements.a > 0) {
        double e_sq = 1.0 - (h_mag * h_mag) / (mu * elements.a);
        if (e_sq < 0) e_sq = 0;  // Numerical safety
        elements.e = std::sqrt(e_sq);
    } else {
        elements.e = 2.0;  // Hyperbolic
    }
    
    // ===========================================================================
    // STEP 5: Compute periapsis and apoapsis
    // ===========================================================================
    
    elements.r_p = elements.a * (1.0 - elements.e);
    elements.r_a = elements.a * (1.0 + elements.e);
    
    // ===========================================================================
    // STEP 6: Compute inclination i = arccos(h_z / |h|)
    // ===========================================================================
    
    if (h_mag > 1e-10) {
        double cos_i = h_vec[2] / h_mag;
        // Clamp to [-1, 1] for numerical safety
        if (cos_i > 1.0) cos_i = 1.0;
        if (cos_i < -1.0) cos_i = -1.0;
        elements.i = std::acos(cos_i);
    } else {
        elements.i = 0;  // Equatorial orbit
    }
    
    // ===========================================================================
    // STEP 7: Compute longitude of ascending node Ω
    // ===========================================================================
    // Ω = arctan2(-h_x, h_y)
    
    elements.Omega = std::atan2(-h_vec[0], h_vec[1]);
    if (elements.Omega < 0) {
        elements.Omega += 2 * 3.14159265358979323846;
    }
    
    // ===========================================================================
    // STEP 8: Compute argument of periapsis ω
    // ===========================================================================
    // ω = arctan2(r_z / sin(i), (r_x*cos(Ω) + r_y*sin(Ω)) / cos(i))
    // More stable form: use eccentricity vector
    
    // Eccentricity vector: e_vec = (1/μ) * (v × h) - r/|r|
    double vh[3] = {
        v[1]*h_vec[2] - v[2]*h_vec[1],
        v[2]*h_vec[0] - v[0]*h_vec[2],
        v[0]*h_vec[1] - v[1]*h_vec[0]
    };
    
    double e_vec[3] = {
        (vh[0] / mu) - (r[0] / r_mag),
        (vh[1] / mu) - (r[1] / r_mag),
        (vh[2] / mu) - (r[2] / r_mag)
    };
    
    // ω = arctan2(e_z / sin(i), (e_x*cos(Ω) + e_y*sin(Ω)))
    double cos_Omega = std::cos(elements.Omega);
    double sin_Omega = std::sin(elements.Omega);
    double sin_i = std::sin(elements.i);
    
    if (std::abs(sin_i) > 1e-10) {
        double numerator = e_vec[2] / sin_i;
        double denominator = e_vec[0]*cos_Omega + e_vec[1]*sin_Omega;
        elements.omega = std::atan2(numerator, denominator);
    } else {
        // Equatorial orbit: set ω = 0
        elements.omega = 0;
    }
    
    if (elements.omega < 0) {
        elements.omega += 2 * 3.14159265358979323846;
    }
    
    // ===========================================================================
    // STEP 9: Compute true anomaly ν
    // ===========================================================================
    // ν = arctan2(r·v̂, h - r·r̂)
    // More stable: ν = arccos((h²/(μ*r) - 1) / e)
    
    if (elements.e > 1e-10) {
        double cos_nu = (h_mag * h_mag / (mu * r_mag) - 1.0) / elements.e;
        // Clamp to [-1, 1] for numerical safety
        if (cos_nu > 1.0) cos_nu = 1.0;
        if (cos_nu < -1.0) cos_nu = -1.0;
        
        // Determine sign of ν from r·v
        double r_dot_v = r[0]*v[0] + r[1]*v[1] + r[2]*v[2];
        elements.nu = std::acos(cos_nu);
        if (r_dot_v < 0) {
            elements.nu = 2 * 3.14159265358979323846 - elements.nu;
        }
    } else {
        // Circular orbit: ν undefined, set to 0
        elements.nu = 0;
    }
    
    return elements;
}
