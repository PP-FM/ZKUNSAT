cmake_minimum_required(VERSION 3.16)

project(emp_private_refutation CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# emp-zk (new, single-threaded, detemplated IO). Pulls in emp-ot::emp-ot and
# emp-tool::emp-tool transitively, along with the arch flags they require.
# Local-dev: point at sibling build trees with
#   -Demp-zk_DIR=/path/emp-zk/build -Demp-ot_DIR=/path/emp-ot/build
#   -Demp-tool_DIR=/path/emp-tool/build
find_package(emp-zk REQUIRED)

# NTL + GMP for the prover's cleartext GF2EX polynomial arithmetic
# (Homebrew keg paths on Apple Silicon; falls back to /usr/local).
find_path(NTL_INCLUDE_DIR NTL/GF2EX.h
    HINTS /opt/homebrew/opt/ntl/include /usr/local/include)
find_library(NTL_LIB ntl HINTS /opt/homebrew/opt/ntl/lib /usr/local/lib)
find_library(GMP_LIB gmp HINTS /opt/homebrew/opt/gmp/lib /usr/local/lib)

add_executable(test main.cpp polynomial.cpp clause.cpp clauseRAM.cpp)
target_include_directories(test PRIVATE ${NTL_INCLUDE_DIR})
target_link_libraries(test PRIVATE emp-zk::emp-zk ${NTL_LIB} ${GMP_LIB})
