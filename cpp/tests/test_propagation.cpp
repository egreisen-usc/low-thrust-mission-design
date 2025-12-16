#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>
#include "../src/constants.h"
#include "../src/propagator.h"
#include "../src/dynamics.h"
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
// DYNAMICS TESTS
// ===========================================================================

void test_gravity_acceleration() {
    std::cout << "\nTest 1: Gravity Acceleration Calculation\n";
    std::cout << "--------------------------------------------\n";
    
    // At Earth's orbit: r = 1.496e8 km, a_gravity = μ/r²
    double r[3] = {1.496e8, 0, 0};
    double mu = MU_SUN;
    
    double a[3];
    computeGravityAccel(r, mu, a);
    
    // Expected: a = -μ/r² in radial direction
    double a_mag = std::sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    double a_expected = mu / (1.496e8 * 1.496e8);
    
    assert_close(a_mag, a_expected, 1e-10, "Gravity acceleration magnitude");
    
    // Direction should be toward Sun (negative r direction)
    if (a[0] < 0) {
        std::cout << "  ✓ PASS: Gravity points toward Sun\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: Gravity should point toward Sun\n";
        tests_failed++;
    }
}

void test_thrust_acceleration() {
    std::cout << "\nTest 2: Thrust Acceleration Calculation\n";
    std::cout << "--------------------------------------------\n";
    
    // Spacecraft velocity in circular orbit
    double v_mag = 29.78;  // km/s
    double v[3] = {0, v_mag, 0};
    double m = 10000;  // kg
    double thrust_mN = 1000;  // mN
    
    double a[3];
    computeThrustAccel(v, m, thrust_mN, a);
    
    // Expected: a = (thrust_mN * 1e-6 / m) * (v / |v|)
    double a_expected_mag = (thrust_mN * 1e-6) / m;
    double a_mag = std::sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    
    assert_close(a_mag, a_expected_mag, 1e-10, "Thrust acceleration magnitude");
    
    // Direction should be parallel to velocity
    double dot_product = a[0]*v[0] + a[1]*v[1] + a[2]*v[2];
    double cos_angle = dot_product / (a_mag * v_mag);
    
    if (std::abs(cos_angle) > 0.9999) {
        std::cout << "  ✓ PASS: Thrust parallel to velocity\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: Thrust should be parallel to velocity\n";
        tests_failed++;
    }
}

void test_total_acceleration() {
    std::cout << "\nTest 3: Total Acceleration (Gravity + Thrust)\n";
    std::cout << "--------------------------------------------\n";
    
    // Create a state at Earth orbit with circular velocity
    double r[3] = {1.496e8, 0, 0};
    double v_circ = std::sqrt(MU_SUN / 1.496e8);
    double v[3] = {0, v_circ, 0};
    
    MissionState state(r[0], r[1], r[2],
                       v[0], v[1], v[2],
                       10000, 0);
    
    double a[3];
    computeAcceleration(state, 1000, MU_SUN, a);
    
    // For circular orbit, gravity provides centripetal acceleration
    // Thrust is small compared to gravity, so total should be dominated by gravity
    double a_mag = std::sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
    double a_gravity = MU_SUN / (1.496e8 * 1.496e8);
    
    // Total should be slightly more than gravity alone (due to thrust component)
    if (a_mag >= a_gravity * 0.99 && a_mag <= a_gravity * 1.01) {
        std::cout << "  ✓ PASS: Total acceleration reasonable\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: Total acceleration not as expected\n";
        tests_failed++;
    }
}

// ===========================================================================
// INTEGRATOR TESTS
// ===========================================================================

void test_rk4_single_step() {
    std::cout << "\nTest 4: RK4 Integrator - Single Step\n";
    std::cout << "--------------------------------------------\n";
    
    // Create initial state in circular orbit
    double r_earth = 1.496e8;
    double v_circ = std::sqrt(MU_SUN / r_earth);
    
    MissionState state(r_earth, 0, 0,
                       0, v_circ, 0,
                       10000, 0);
    
    RK4Propagator rk4;
    double dt = 10000;  // 10000 seconds
    
    rk4.step(state, dt, 1000, 2750, MU_SUN, G0);
    
    // After one step, position should have changed
    if (std::abs(state.r[0] - r_earth) > 0.1) {
        std::cout << "  ✓ PASS: Position changed after integration step\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: Position should change after step\n";
        tests_failed++;
    }
    
    // Velocity should have changed due to thrust and gravity
    if (std::abs(state.v[1] - v_circ) > 0.0001) {
        std::cout << "  ✓ PASS: Velocity changed after integration step\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: Velocity should change after step\n";
        tests_failed++;
    }
    
    // Time should have advanced
    if (std::abs(state.t - dt) < 1.0) {
        std::cout << "  ✓ PASS: Time advanced correctly\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: Time not advanced correctly\n";
        tests_failed++;
    }
}

void test_euler_single_step() {
    std::cout << "\nTest 5: Euler Integrator - Single Step\n";
    std::cout << "--------------------------------------------\n";
    
    // Create initial state in circular orbit
    double r_earth = 1.496e8;
    double v_circ = std::sqrt(MU_SUN / r_earth);
    
    MissionState state(r_earth, 0, 0,
                       0, v_circ, 0,
                       10000, 0);
    
    EulerPropagator euler;
    double dt = 10000;
    
    euler.step(state, dt, 1000, 2750, MU_SUN, G0);
    
    // After one step, position should have changed
    if (std::abs(state.r[0] - r_earth) > 0.1) {
        std::cout << "  ✓ PASS: Position changed after integration step\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: Position should change after step\n";
        tests_failed++;
    }
    
    // Time should have advanced
    if (std::abs(state.t - dt) < 1.0) {
        std::cout << "  ✓ PASS: Time advanced correctly\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: Time not advanced correctly\n";
        tests_failed++;
    }
}

void test_rk4_vs_euler_accuracy() {
    std::cout << "\nTest 6: RK4 vs Euler Accuracy Comparison\n";
    std::cout << "--------------------------------------------\n";
    
    // Create two identical states
    double r_earth = 1.496e8;
    double v_circ = std::sqrt(MU_SUN / r_earth);
    
    MissionState state_rk4(r_earth, 0, 0, 0, v_circ, 0, 10000, 0);
    MissionState state_euler(r_earth, 0, 0, 0, v_circ, 0, 10000, 0);
    
    RK4Propagator rk4;
    EulerPropagator euler;
    double dt = 10000;
    
    // Take 10 steps with each integrator
    for (int i = 0; i < 10; i++) {
        rk4.step(state_rk4, dt, 1000, 2750, MU_SUN, G0);
        euler.step(state_euler, dt, 1000, 2750, MU_SUN, G0);
    }
    
    // RK4 should be more accurate (smaller position difference)
    double pos_diff_rk4 = std::sqrt((state_rk4.r[0]-r_earth)*(state_rk4.r[0]-r_earth) +
                                     (state_rk4.r[1]*state_rk4.r[1]));
    double pos_diff_euler = std::sqrt((state_euler.r[0]-r_earth)*(state_euler.r[0]-r_earth) +
                                       (state_euler.r[1]*state_euler.r[1]));
    
    std::cout << "    RK4 position deviation:   " << std::scientific << pos_diff_rk4 << " km\n";
    std::cout << "    Euler position deviation: " << pos_diff_euler << " km\n";
    
    if (pos_diff_rk4 < pos_diff_euler) {
        std::cout << "  ✓ PASS: RK4 more accurate than Euler\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: RK4 should be more accurate than Euler\n";
        tests_failed++;
    }
}

// ===========================================================================
// CONSERVATION TESTS
// ===========================================================================

void test_energy_conservation_coasting() {
    std::cout << "\nTest 7: Energy Conservation - Coasting Orbit\n";
    std::cout << "--------------------------------------------\n";
    
    // Create circular orbit and coast (no thrust)
    double r_earth = 1.496e8;
    double v_circ = std::sqrt(MU_SUN / r_earth);
    
    MissionState state(r_earth, 0, 0, 0, v_circ, 0, 10000, 0);
    
    // Initial specific orbital energy
    double E_initial = (v_circ*v_circ/2.0) - (MU_SUN / r_earth);
    
    RK4Propagator rk4;
    double dt = 10000;
    
    // Coast for 100 steps
    for (int i = 0; i < 100; i++) {
        rk4.step(state, dt, 0, 2750, MU_SUN, G0);  // No thrust (0 mN)
    }
    
    // Compute final specific orbital energy
    double r_mag = state.radius();
    double v_mag = state.speed();
    double E_final = (v_mag*v_mag/2.0) - (MU_SUN / r_mag);
    
    // Energy should be conserved (relative error < 0.1%)
    double rel_energy_error = std::abs(E_final - E_initial) / std::abs(E_initial);
    
    std::cout << "    Initial energy: " << std::scientific << E_initial << " km²/s²\n";
    std::cout << "    Final energy:   " << E_final << " km²/s²\n";
    std::cout << "    Relative error: " << std::fixed << std::setprecision(6) 
              << (rel_energy_error * 100) << "%\n";
    
    if (rel_energy_error < 0.001) {
        std::cout << "  ✓ PASS: Energy conserved in coasting orbit\n";
        tests_passed++;
    } else {
        std::cout << "  ✗ FAIL: Energy not conserved adequately\n";
        tests_failed++;
    }
}

// ===========================================================================
// MAIN TEST RUNNER
// ===========================================================================

int main() {
    std::cout << "\n";
    std::cout << "=====================================================\n";
    std::cout << "PROPAGATION AND DYNAMICS TEST SUITE\n";
    std::cout << "=====================================================\n";
    
    // Dynamics tests
    test_gravity_acceleration();
    test_thrust_acceleration();
    test_total_acceleration();
    
    // Integrator tests
    test_rk4_single_step();
    test_euler_single_step();
    test_rk4_vs_euler_accuracy();
    
    // Conservation tests
    test_energy_conservation_coasting();
    
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
