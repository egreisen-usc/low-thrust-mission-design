#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

// ===========================================================================
// PHYSICAL CONSTANTS
// ===========================================================================

// Sun's gravitational parameter (km³/s²)
constexpr double MU_SUN = 1.32712440018e11;

// Gravitational acceleration constant (km/s²)
constexpr double G0 = 9.81e-3;

// ===========================================================================
// ORBITAL RADII (km) - heliocentric distances
// ===========================================================================

constexpr double R_MERCURY = 5.7909e7;      // 0.3871 AU
constexpr double R_VENUS = 1.08208e8;       // 0.7233 AU
constexpr double R_EARTH = 1.496e8;         // 1.0000 AU
constexpr double R_MARS = 2.2794e8;         // 1.5237 AU
constexpr double R_JUPITER = 7.7857e8;      // 5.2034 AU
constexpr double R_SATURN = 1.4336e9;       // 9.5371 AU
constexpr double R_URANUS = 2.8725e9;       // 19.191 AU
constexpr double R_NEPTUNE = 4.4951e9;      // 30.069 AU
constexpr double R_PLUTO = 5.9130e9;        // 39.482 AU (dwarf planet)

// ===========================================================================
// NUMERICAL CONSTANTS
// ===========================================================================

constexpr int MAX_STEPS = 10000000;
constexpr double COAST_THRESHOLD = 0.999;
constexpr double KEPLER_TOLERANCE = 1e-12;

// ===========================================================================
// CELESTIAL BODY ENUM
// ===========================================================================

enum class CelestialBody {
    MERCURY = 0,
    VENUS = 1,
    EARTH = 2,
    MARS = 3,
    JUPITER = 4,
    SATURN = 5,
    URANUS = 6,
    NEPTUNE = 7,
    PLUTO = 8
};

// ===========================================================================
// HELPER FUNCTIONS
// ===========================================================================

/// Convert enum to orbital radius (km)
inline double getOrbitalRadius(CelestialBody body) {
    switch (body) {
        case CelestialBody::MERCURY:  return R_MERCURY;
        case CelestialBody::VENUS:    return R_VENUS;
        case CelestialBody::EARTH:    return R_EARTH;
        case CelestialBody::MARS:     return R_MARS;
        case CelestialBody::JUPITER:  return R_JUPITER;
        case CelestialBody::SATURN:   return R_SATURN;
        case CelestialBody::URANUS:   return R_URANUS;
        case CelestialBody::NEPTUNE:  return R_NEPTUNE;
        case CelestialBody::PLUTO:    return R_PLUTO;
        default:                      return R_EARTH;
    }
}

/// Convert enum to body name (for printing)
inline const char* getBodyName(CelestialBody body) {
    switch (body) {
        case CelestialBody::MERCURY:  return "Mercury";
        case CelestialBody::VENUS:    return "Venus";
        case CelestialBody::EARTH:    return "Earth";
        case CelestialBody::MARS:     return "Mars";
        case CelestialBody::JUPITER:  return "Jupiter";
        case CelestialBody::SATURN:   return "Saturn";
        case CelestialBody::URANUS:   return "Uranus";
        case CelestialBody::NEPTUNE:  return "Neptune";
        case CelestialBody::PLUTO:    return "Pluto";
        default:                      return "Unknown";
    }
}

/// Parse body name from string (case-insensitive)
inline CelestialBody parseBodyName(const std::string& name) {
    if (name == "Mercury" || name == "mercury")   return CelestialBody::MERCURY;
    if (name == "Venus" || name == "venus")       return CelestialBody::VENUS;
    if (name == "Earth" || name == "earth")       return CelestialBody::EARTH;
    if (name == "Mars" || name == "mars")         return CelestialBody::MARS;
    if (name == "Jupiter" || name == "jupiter")   return CelestialBody::JUPITER;
    if (name == "Saturn" || name == "saturn")     return CelestialBody::SATURN;
    if (name == "Uranus" || name == "uranus")     return CelestialBody::URANUS;
    if (name == "Neptune" || name == "neptune")   return CelestialBody::NEPTUNE;
    if (name == "Pluto" || name == "pluto")       return CelestialBody::PLUTO;
    return CelestialBody::EARTH;  // default
}

#endif // CONSTANTS_H
