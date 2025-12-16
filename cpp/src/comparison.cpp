#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <cmath>
#include "comparison.h"

// ===========================================================================
// COMPARISON ENGINE IMPLEMENTATION
// ===========================================================================

void MissionComparison::addMission(const MissionResult& result) {
    missions.push_back(result);
}

void MissionComparison::computeMetrics() {
    for (auto& mission : missions) {
        // Payload fraction: remaining mass / initial mass
        mission.payload_fraction = mission.final_mass_kg / mission.initial_mass_kg;
        
        // Fuel efficiency: how much delta-V per kg of fuel burned
        if (mission.propellant_consumed_kg > 1e-10) {
            mission.fuel_efficiency = mission.total_delta_v_km_s / mission.propellant_consumed_kg;
        } else {
            mission.fuel_efficiency = 0;
        }
        
        // Effective ISP from delta-V equation: Δv = ISP * g0 * ln(m0/mf)
        // Solving for ISP: ISP = Δv / (g0 * ln(m0/mf))
        double G0 = 9.80665e-3;  // km/s² (converted from 9.80665 m/s²)
        if (mission.initial_mass_kg > mission.final_mass_kg) {
            double mass_ratio = mission.initial_mass_kg / mission.final_mass_kg;
            mission.specific_impulse_achieved = mission.total_delta_v_km_s / 
                                               (G0 * std::log(mass_ratio));
        } else {
            mission.specific_impulse_achieved = 0;
        }
        
        // Transfer efficiency: how close final apoapsis is to target
        // Target apoapsis varies by destination:
        // - Mars: 2.279e8 km
        // - Venus: 1.082e8 km
        // - Jupiter: 7.785e8 km
        double target_apoapsis = 0;
        if (mission.arrival_body == "Mars") {
            target_apoapsis = 2.279e8;
        } else if (mission.arrival_body == "Venus") {
            target_apoapsis = 1.082e8;
        } else if (mission.arrival_body == "Jupiter") {
            target_apoapsis = 7.785e8;
        }
        
        if (target_apoapsis > 1e-10) {
            mission.transfer_efficiency = (mission.final_apoapsis_km / target_apoapsis) * 100.0;
        } else {
            mission.transfer_efficiency = 0;
        }
    }
}

void MissionComparison::writeComparisonCSV(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open output file: " << filename << "\n";
        return;
    }
    
    // Write header
    file << "Mission,Thruster,From,To,"
         << "FlightTime(days),DeltaV(km/s),FuelConsumed(kg),FinalMass(kg),"
         << "Apoapsis(km),Periapsis(km),Eccentricity,SemiMajorAxis(km),"
         << "PayloadFraction,EffectiveISP(s),FuelEfficiency(km/s/kg),TransferEfficiency(%)\n";
    
    // Write data rows
    for (const auto& mission : missions) {
        file << std::fixed << std::setprecision(2)
             << mission.mission_name << ","
             << mission.thruster_name << ","
             << mission.departure_body << ","
             << mission.arrival_body << ","
             << mission.flight_time_days << ","
             << mission.total_delta_v_km_s << ","
             << mission.propellant_consumed_kg << ","
             << mission.final_mass_kg << ","
             << std::scientific << std::setprecision(3)
             << mission.final_apoapsis_km << ","
             << mission.final_periapsis_km << ","
             << std::fixed << std::setprecision(6)
             << mission.final_eccentricity << ","
             << std::scientific << std::setprecision(3)
             << mission.final_semi_major_axis_km << ","
             << std::fixed << std::setprecision(4)
             << mission.payload_fraction << ","
             << std::setprecision(1)
             << mission.specific_impulse_achieved << ","
             << std::setprecision(3)
             << mission.fuel_efficiency << ","
             << std::setprecision(1)
             << mission.transfer_efficiency << "\n";
    }
    
    file.close();
    std::cout << "Comparison results saved to: " << filename << "\n";
}

void MissionComparison::printSummary() {
    if (missions.empty()) {
        std::cout << "No missions to compare.\n";
        return;
    }
    
    std::cout << "\n";
    std::cout << "=====================================================\n";
    std::cout << "MISSION COMPARISON SUMMARY\n";
    std::cout << "=====================================================\n";
    std::cout << "Total missions analyzed: " << missions.size() << "\n\n";
    
    // Group by thruster type
    std::cout << "Results by Thruster Type:\n";
    std::cout << "---\n";
    
    std::vector<std::string> thrusters;
    for (const auto& mission : missions) {
        if (std::find(thrusters.begin(), thrusters.end(), mission.thruster_name) == thrusters.end()) {
            thrusters.push_back(mission.thruster_name);
        }
    }
    
    for (const auto& thruster : thrusters) {
        auto thruster_missions = getMissionsByThruster(thruster);
        
        double avg_time = 0, avg_delta_v = 0, avg_fuel = 0;
        for (const auto& m : thruster_missions) {
            avg_time += m.flight_time_days;
            avg_delta_v += m.total_delta_v_km_s;
            avg_fuel += m.propellant_consumed_kg;
        }
        avg_time /= thruster_missions.size();
        avg_delta_v /= thruster_missions.size();
        avg_fuel /= thruster_missions.size();
        
        std::cout << "  " << thruster << ":\n";
        std::cout << "    Missions: " << thruster_missions.size() << "\n";
        std::cout << "    Avg flight time: " << std::fixed << std::setprecision(1) 
                  << avg_time << " days\n";
        std::cout << "    Avg delta-V: " << std::setprecision(2) << avg_delta_v << " km/s\n";
        std::cout << "    Avg fuel consumed: " << std::setprecision(0) << avg_fuel << " kg\n";
    }
    
    std::cout << "\n";
    
    // Group by target body
    std::cout << "Results by Target Body:\n";
    std::cout << "---\n";
    
    std::vector<std::string> targets;
    for (const auto& mission : missions) {
        if (std::find(targets.begin(), targets.end(), mission.arrival_body) == targets.end()) {
            targets.push_back(mission.arrival_body);
        }
    }
    
    for (const auto& target : targets) {
        auto target_missions = getMissionsByTarget(target);
        
        double min_time = 1e10, min_delta_v = 1e10;
        for (const auto& m : target_missions) {
            min_time = std::min(min_time, m.flight_time_days);
            min_delta_v = std::min(min_delta_v, m.total_delta_v_km_s);
        }
        
        std::cout << "  To " << target << ":\n";
        std::cout << "    Fastest transfer: " << std::fixed << std::setprecision(1) 
                  << min_time << " days\n";
        std::cout << "    Minimum delta-V: " << std::setprecision(2) << min_delta_v << " km/s\n";
    }
    
    std::cout << "\n";
}

MissionResult MissionComparison::findBestMission(const std::string& metric) {
    if (missions.empty()) {
        std::cerr << "No missions to compare.\n";
        return MissionResult();
    }
    
    if (metric == "shortest_time") {
        return *std::min_element(missions.begin(), missions.end(),
            [](const MissionResult& a, const MissionResult& b) {
                return a.flight_time_days < b.flight_time_days;
            });
    } else if (metric == "lowest_delta_v") {
        return *std::min_element(missions.begin(), missions.end(),
            [](const MissionResult& a, const MissionResult& b) {
                return a.total_delta_v_km_s < b.total_delta_v_km_s;
            });
    } else if (metric == "least_fuel") {
        return *std::min_element(missions.begin(), missions.end(),
            [](const MissionResult& a, const MissionResult& b) {
                return a.propellant_consumed_kg < b.propellant_consumed_kg;
            });
    } else if (metric == "most_efficient") {
        return *std::max_element(missions.begin(), missions.end(),
            [](const MissionResult& a, const MissionResult& b) {
                return a.payload_fraction < b.payload_fraction;
            });
    } else {
        std::cerr << "Unknown metric: " << metric << "\n";
        return MissionResult();
    }
}

std::vector<MissionResult> MissionComparison::getMissionsByThruster(const std::string& thruster) {
    std::vector<MissionResult> result;
    for (const auto& mission : missions) {
        if (mission.thruster_name == thruster) {
            result.push_back(mission);
        }
    }
    return result;
}

std::vector<MissionResult> MissionComparison::getMissionsByTarget(const std::string& target) {
    std::vector<MissionResult> result;
    for (const auto& mission : missions) {
        if (mission.arrival_body == target) {
            result.push_back(mission);
        }
    }
    return result;
}
