cmake_minimum_required(VERSION 3.16)
project(LowThrustMissionDesign)

# ===========================================================================
# COMPILER CONFIGURATION (Fedora/Linux optimized)
# ===========================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /O2")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

# ===========================================================================
# OUTPUT DIRECTORIES
# ===========================================================================

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ===========================================================================
# TESTING
# ===========================================================================

enable_testing()

# ===========================================================================
# SUBDIRECTORIES
# ===========================================================================

add_subdirectory(cpp)

# ===========================================================================
# BUILD SUMMARY
# ===========================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "Low-Thrust Mission Design Tool")
message(STATUS "========================================")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Binary Dir: ${CMAKE_BINARY_DIR}/bin")
message(STATUS "========================================")
message(STATUS "")
