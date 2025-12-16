# ===========================================================================
# C++ PROPAGATOR BUILD CONFIGURATION
# ===========================================================================

# Find yaml-cpp package
find_package(yaml-cpp REQUIRED)

# ===========================================================================
# MAIN EXECUTABLE
# ===========================================================================

set(PROPAGATOR_SOURCES
    src/main.cpp
    src/propagator.cpp
    src/dynamics.cpp
    src/orbital_elements.cpp
)

add_executable(propagate_trajectory ${PROPAGATOR_SOURCES})

# ===========================================================================
# INCLUDE DIRECTORIES
# ===========================================================================

target_include_directories(propagate_trajectory 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${YAML_CPP_INCLUDE_DIR}
)

# ===========================================================================
# COMPILER OPTIONS
# ===========================================================================

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(propagate_trajectory PRIVATE -Wall -Wextra)
endif()

# ===========================================================================
# LINK LIBRARIES
# ===========================================================================

# Link yaml-cpp for configuration file parsing
target_link_libraries(propagate_trajectory PRIVATE yaml-cpp)

# Link math library on Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(propagate_trajectory PRIVATE m)
endif()

# ===========================================================================
# TESTS
# ===========================================================================

# Test 1: Kepler Solver
add_executable(test_kepler 
    tests/test_kepler.cpp 
    src/orbital_elements.cpp
)
target_include_directories(test_kepler PRIVATE src)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_kepler PRIVATE -Wall -Wextra)
endif()
if(UNIX AND NOT APPLE)
    target_link_libraries(test_kepler PRIVATE m)
endif()
add_test(NAME TestKepler COMMAND test_kepler)

# Test 2: Propagation
add_executable(test_propagation
    tests/test_propagation.cpp
    src/propagator.cpp
    src/dynamics.cpp
    src/orbital_elements.cpp
)
target_include_directories(test_propagation PRIVATE src)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_propagation PRIVATE -Wall -Wextra)
endif()
if(UNIX AND NOT APPLE)
    target_link_libraries(test_propagation PRIVATE m)
endif()
add_test(NAME TestPropagation COMMAND test_propagation)
