# ===========================================================================
# SOURCE FILES FOR MAIN PROPAGATOR EXECUTABLE
# ===========================================================================

set(PROPAGATOR_SOURCES
    src/main.cpp
    src/propagator.cpp
    src/dynamics.cpp
    src/orbital_elements.cpp
)

# ===========================================================================
# CREATE EXECUTABLE
# ===========================================================================

add_executable(propagate_trajectory ${PROPAGATOR_SOURCES})

# ===========================================================================
# INCLUDE DIRECTORIES (where to find headers)
# ===========================================================================

target_include_directories(propagate_trajectory 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ===========================================================================
# COMPILER OPTIONS FOR THIS TARGET
# ===========================================================================

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(propagate_trajectory PRIVATE 
        -Wall -Wextra -Wpedantic -Werror=format
    )
endif()

# ===========================================================================
# LINK OPTIONS
# ===========================================================================

# Link against standard math library on Linux
if(UNIX AND NOT APPLE)
    target_link_libraries(propagate_trajectory PRIVATE m)
endif()
