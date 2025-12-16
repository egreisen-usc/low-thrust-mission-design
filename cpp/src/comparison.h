#ifndef COMPARISON_H
#define COMPARISON_H

#include <string>
#include <vector>

// ===========================================================================
// MISSION RESULT STRUCTURE
// ===========================================================================
// Stores the outcome of a single mission for later comparison

struct MissionResult {
    // Mission identification
    std::string mission_name;
    std::string thruster_name;
    std::string departure_body;
    std::string arrival_body;
    
    // Mission outcomes
    double flight_time_days;          // Duration of thrust phase (days)
    double total_delta_v_km_s;        // Total velocity change (km/s)
    double propellant_consumed_kg;    // Fuel burned (kg)
    double final_mass_kg;             // Remaining mass (kg)
    double initial_mass_kg;           // Starting mass (kg)
    
    // Orbital elements at coast activation
    double final_apoapsis_km;         // Apoapsis when coasting begins (km)
    double final_periapsis_km;        // Periapsis when coasting begins (km)
    double final_eccentricity;        // Eccentricity at coast (dimensionless)
    double final_semi_major_axis_km;  // Semi-major axis at coast (km)
    
    // Derived metrics
    double payload_fraction;          // final_mass / initial_mass (0 to 1)
    double specific_impulse_achieved; // Effective ISP from delta-V (s)
    double fuel_efficiency;           // delta-V per kg fuel (km/s/kg)
    double transfer_efficiency;       // How close apoapsis is to target (%)
    
    /// Constructor
    MissionResult() : flight_time_days(0), total_delta_v_km_s(0),
                      propellant_consumed_kg(0), final_mass_kg(0),
                      initial_mass_kg(0), final_apoapsis_km(0),
                      final_periapsis_km(0), final_eccentricity(0),
                      final_semi_major_axis_km(0), payload_fraction(0),
                      specific_impulse_achieved(0), fuel_efficiency(0),
                      transfer_efficiency(0) {}
};

// ===========================================================================
// COMPARISON ENGINE
// ===========================================================================

class MissionComparison {
public:
    /// Add a mission result for comparison
    void addMission(const MissionResult& result);
    
    /// Compute derived metrics (payload fraction, efficiency, etc)
    void computeMetrics();
    
    /// Write all results to CSV file
    void writeComparisonCSV(const std::string& filename);
    
    /// Write summary statistics to console
    void printSummary();
    
    /// Find best mission by metric (e.g., "shortest_time", "most_fuel_efficient")
    MissionResult findBestMission(const std::string& metric);
    
    /// Get all results for a specific thruster type
    std::vector<MissionResult> getMissionsByThruster(const std::string& thruster);
    
    /// Get all results for a specific target body
    std::vector<MissionResult> getMissionsByTarget(const std::string& target);
    
private:
    std::vector<MissionResult> missions;
};

#endif // COMPARISON_H
