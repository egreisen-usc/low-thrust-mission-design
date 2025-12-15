# ===========================================================================
# TEST 1: KEPLER SOLVER
# ===========================================================================

add_executable(test_kepler 
    test_kepler.cpp 
    ../src/orbital_elements.cpp
)

target_include_directories(test_kepler 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_kepler PRIVATE -Wall -Wextra)
endif()

# Link math library on Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(test_kepler PRIVATE m)
endif()

add_test(NAME TestKepler COMMAND test_kepler)

# ===========================================================================
# TEST 2: PROPAGATION
# ===========================================================================

add_executable(test_propagation
    test_propagation.cpp
    ../src/propagator.cpp
    ../src/dynamics.cpp
    ../src/orbital_elements.cpp
)

target_include_directories(test_propagation
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_propagation PRIVATE -Wall -Wextra)
endif()

# Link math library on Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(test_propagation PRIVATE m)
endif()

add_test(NAME TestPropagation COMMAND test_propagation)

# ===========================================================================
# TEST SUMMARY
# ===========================================================================

message(STATUS "Tests registered:")
message(STATUS "  - TestKepler")
message(STATUS "  - TestPropagation")
