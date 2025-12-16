#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>
#include "../src/orbital_elements.h"

// ===========================================================================
// TEST FRAMEWORK HELPERS
// ===========================================================================

int tests_passed = 0;
int tests_failed = 0;

void assert_close(double actual, double expected, double tolerance, 
                  const std::string& test_name) {
    double error = std::abs(actual - expected);
    double rel_error = std::abs(expected) > 1e-10 ? 
                       error / std::abs(expected) : error;
    
    if (rel_error <= tolerance) {
        std::cout << "  ✓ PASS: " << test_name << "\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: " << test_name << "\n";
        std::cout << "    Expected: " << std::scientific << expected << "\n";
        std::cout << "    Actual:   " << actual << "\n";
        std::cout << "    Error:    " << rel_error * 100 << "%\n";
        tests_failed++;
    }
}

// ===========================================================================
// KEPLER SOLVER TESTS
// ===========================================================================

void test_kepler_circular_orbit() {
    std::cout << "\nTest 1: Kepler Solver - Circular Orbit (e = 0)\n";
    std::cout << "--------------------------------------------\n";
    
    // For circular orbit, E = M (solution is trivial)
    double M = 1.5;
    double e = 0.0;
    double E = solveKeplersEquation(M, e);
    
    // Verify Kepler equation: M = E - e*sin(E)
    double M_check = E - e * std::sin(E);
    assert_close(M_check, M, 1e-10, "Kepler equation satisfied (e=0)");
    
    // For circular orbit, E should equal M
    assert_close(E, M, 1e-12, "E = M for circular orbit");
}

void test_kepler_elliptical_orbit_low_e() {
    std::cout << "\nTest 2: Kepler Solver - Elliptical Orbit (e = 0.3)\n";
    std::cout << "--------------------------------------------\n";
    
    // Test case: e = 0.3, M = 1.0
    double M = 1.0;
    double e = 0.3;
    double E = solveKeplersEquation(M, e);
    
    // Verify Kepler equation: M = E - e*sin(E)
    double M_check = E - e * std::sin(E);
    assert_close(M_check, M, 1e-12, "Kepler equation satisfied (e=0.3, M=1.0)");
    
    // E should be close to M for small e
    assert_close(E, M, 0.01, "E ≈ M for low eccentricity");
}

void test_kepler_elliptical_orbit_moderate_e() {
    std::cout << "\nTest 3: Kepler Solver - Elliptical Orbit (e = 0.5)\n";
    std::cout << "--------------------------------------------\n";
    
    // Test case: e = 0.5, M = 3.0
    double M = 3.0;
    double e = 0.5;
    double E = solveKeplersEquation(M, e);
    
    // Verify Kepler equation
    double M_check = E - e * std::sin(E);
    assert_close(M_check, M, 1e-12, "Kepler equation satisfied (e=0.5, M=3.0)");
    
    // E should be significantly different from M
    double E_M_diff = std::abs(E - M);
    if (E_M_diff > 0.1) {
        std::cout << "  ✓ PASS: E differs from M for moderate eccentricity (diff=" 
                  << E_M_diff << ")\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: E should differ from M for e=0.5\n";
        tests_failed++;
    }
}

void test_kepler_elliptical_orbit_high_e() {
    std::cout << "\nTest 4: Kepler Solver - Elliptical Orbit (e = 0.9)\n";
    std::cout << "--------------------------------------------\n";
    
    // Test case: e = 0.9, M = 1.57 (near π/2)
    double M = 1.57;
    double e = 0.9;
    double E = solveKeplersEquation(M, e);
    
    // Verify Kepler equation (should be very accurate even for high e)
    double M_check = E - e * std::sin(E);
    assert_close(M_check, M, 1e-11, "Kepler equation satisfied (e=0.9, M=1.57)");
}

void test_kepler_multiple_mean_anomalies() {
    std::cout << "\nTest 5: Kepler Solver - Multiple Mean Anomalies\n";
    std::cout << "--------------------------------------------\n";
    
    double e = 0.2;
    double M_values[] = {0.0, 0.5, 1.0, 1.5, 2.0, 3.0, 4.0, 5.0};
    
    bool all_pass = true;
    for (int i = 0; i < 8; i++) {
        double M = M_values[i];
        double E = solveKeplersEquation(M, e);
        double M_check = E - e * std::sin(E);
        
        double error = std::abs(M_check - M);
        if (error > 1e-11) {
            all_pass = false;
        }
    }
    
    if (all_pass) {
        std::cout << "  ✓ PASS: All mean anomalies solved accurately\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: Some mean anomalies not solved accurately\n";
        tests_failed++;
    }
}

// ===========================================================================
// ECCENTRIC TO TRUE ANOMALY CONVERSION TESTS
// ===========================================================================

void test_eccentric_to_true_circular() {
    std::cout << "\nTest 6: Eccentric to True Anomaly - Circular Orbit (e = 0)\n";
    std::cout << "--------------------------------------------\n";
    
    // For circular orbit, ν = E
    double E = 1.5;
    double e = 0.0;
    double nu = eccentricToTrueAnomaly(E, e);
    
    assert_close(nu, E, 1e-12, "ν = E for circular orbit");
}

void test_eccentric_to_true_elliptical() {
    std::cout << "\nTest 7: Eccentric to True Anomaly - Elliptical Orbit (e = 0.5)\n";
    std::cout << "--------------------------------------------\n";
    
    // For elliptical orbit with e = 0.5, E = π/2 should give specific ν
    double E = 3.14159265358979323846 / 2.0;  // π/2
    double e = 0.5;
    double nu = eccentricToTrueAnomaly(E, e);
    
    // ν should be greater than E for elliptical orbits
    if (nu > E) {
        std::cout << "  ✓ PASS: ν > E for elliptical orbit (e=0.5)\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: ν should be > E for e=0.5, E=π/2\n";
        tests_failed++;
    }
    
    // Verify result is in [0, 2π)
    if (nu >= 0 && nu < 2 * 3.14159265358979323846) {
        std::cout << "  ✓ PASS: ν is in valid range [0, 2π)\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: ν out of valid range\n";
        tests_failed++;
    }
}

void test_eccentric_to_true_periapsis() {
    std::cout << "\nTest 8: Eccentric to True Anomaly - At Periapsis (E = 0)\n";
    std::cout << "--------------------------------------------\n";
    
    // At periapsis, E = 0, so ν = 0
    double E = 0.0;
    double e = 0.5;
    double nu = eccentricToTrueAnomaly(E, e);
    
    assert_close(nu, 0.0, 1e-12, "ν = 0 at periapsis");
}

void test_eccentric_to_true_apoapsis() {
    std::cout << "\nTest 9: Eccentric to True Anomaly - At Apoapsis (E = π)\n";
    std::cout << "--------------------------------------------\n";
    
    // At apoapsis, E = π, so ν = π
    double E = 3.14159265358979323846;  // π
    double e = 0.5;
    double nu = eccentricToTrueAnomaly(E, e);
    
    assert_close(nu, 3.14159265358979323846, 1e-12, "ν = π at apoapsis");
}

// ===========================================================================
// ORBITAL ELEMENTS COMPUTATION TESTS
// ===========================================================================

void test_orbital_elements_circular_orbit() {
    std::cout << "\nTest 10: Orbital Elements - Circular Orbit\n";
    std::cout << "--------------------------------------------\n";
    
    // Create a circular orbit at 1.496e8 km (Earth orbit)
    double r[3] = {1.496e8, 0, 0};
    double mu = 1.327e11;
    double v_circ = std::sqrt(mu / 1.496e8);
    double v[3] = {0, v_circ, 0};
    
    OrbitalElements elements = computeOrbitalElements(r, v, mu);
    
    // For circular orbit:
    // - a = r
    // - e = 0
    // - r_p = r_a = r
    
    assert_close(elements.a, 1.496e8, 1e-6, "Semi-major axis = r");
    assert_close(elements.e, 0.0, 1e-6, "Eccentricity = 0");
    assert_close(elements.r_p, 1.496e8, 1e-6, "Periapsis = r");
    assert_close(elements.r_a, 1.496e8, 1e-6, "Apoapsis = r");
}

void test_orbital_elements_elliptical_orbit() {
    std::cout << "\nTest 11: Orbital Elements - Elliptical Orbit\n";
    std::cout << "--------------------------------------------\n";
    
    // Create an elliptical orbit with a = 1.5e8 km, e = 0.1
    // At periapsis: r = a(1-e) = 1.35e8 km
    // Periapsis velocity: v = sqrt(μ(2/r - 1/a))
    
    double a = 1.5e8;
    double e = 0.1;
    double mu = 1.327e11;
    
    double r_p = a * (1 - e);
    double v_p = std::sqrt(mu * (2.0 / r_p - 1.0 / a));
    
    double r[3] = {r_p, 0, 0};
    double v[3] = {0, v_p, 0};
    
    OrbitalElements elements = computeOrbitalElements(r, v, mu);
    
    assert_close(elements.a, a, 1e-6, "Semi-major axis = 1.5e8 km");
    assert_close(elements.e, e, 1e-6, "Eccentricity = 0.1");
    assert_close(elements.r_p, r_p, 1e-3, "Periapsis computed correctly");
}

void test_orbital_elements_inclination() {
    std::cout << "\nTest 12: Orbital Elements - Inclination\n";
    std::cout << "--------------------------------------------\n";
    
    // Create an inclined circular orbit (30 degrees inclination)
    double r_mag = 1.496e8;
    double mu = 1.327e11;
    double v_circ = std::sqrt(mu / r_mag);
    
    double inclination = 30.0 * 3.14159265358979323846 / 180.0;  // Convert to radians
    
    // Position in orbital plane
    double r[3] = {r_mag * std::cos(inclination), r_mag * std::sin(inclination), 0};
    // Velocity perpendicular to radius
    double v[3] = {-v_circ * std::sin(inclination), v_circ * std::cos(inclination), 0};
    
    OrbitalElements elements = computeOrbitalElements(r, v, mu);
    
    // Convert inclination back to degrees for comparison
    double i_degrees = elements.i * 180.0 / 3.14159265358979323846;
    assert_close(i_degrees, 30.0, 0.1, "Inclination = 30 degrees");
}

// ===========================================================================
// MAIN TEST RUNNER
// ===========================================================================

int main() {
    std::cout << "\n";
    std::cout << "=====================================================\n";
    std::cout << "KEPLER SOLVER AND ORBITAL ELEMENTS TEST SUITE\n";
    std::cout << "=====================================================\n";
    
    // Kepler solver tests
    test_kepler_circular_orbit();
    test_kepler_elliptical_orbit_low_e();
    test_kepler_elliptical_orbit_moderate_e();
    test_kepler_elliptical_orbit_high_e();
    test_kepler_multiple_mean_anomalies();
    
    // Eccentric to true anomaly tests
    test_eccentric_to_true_circular();
    test_eccentric_to_true_elliptical();
    test_eccentric_to_true_periapsis();
    test_eccentric_to_true_apoapsis();
    
    // Orbital elements tests
    test_orbital_elements_circular_orbit();
    test_orbital_elements_elliptical_orbit();
    test_orbital_elements_inclination();
    
    // Summary
    std::cout << "\n";
    std::cout << "=====================================================\n";
    std::cout << "TEST RESULTS\n";
    std::cout << "=====================================================\n";
    std::cout << "  Tests passed: " << tests_passed << "\n";
    std::cout << "  Tests failed: " << tests_failed << "\n";
    std::cout << "  Total tests:  " << (tests_passed + tests_failed) << "\n";
    
    if (tests_failed == 0) {
        std::cout << "\n  ✓ ALL TESTS PASSED\n";
        std::cout << "=====================================================\n\n";
        return 0;
    } else {
        std::cout << "\n  ✗ SOME TESTS FAILED\n";
        std::cout << "=====================================================\n\n";
        return 1;
    }
}
