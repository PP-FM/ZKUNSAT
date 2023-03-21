cmake_minimum_required(VERSION 3.10)

project(emp_private_refutation)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-msse4.1 -mrdseed -maes -mpclmul -O3 -pthread -w -Wl,-rpath -Wl,LIBDIR")

include_directories(include /home/ubuntu/emp-tool)
include_directories(include /home/ubuntu/emp-zk)


#
#enable_testing()
#
#add_subdirectory(src)
#add_subdirectory(test)

# unit
#add_executable(test test.cpp clause.cpp f2kpoly.cpp xf2kpoly.cpp polynomial.cpp clauseRAM.cpp proof.cpp)
#target_link_libraries(test PUBLIC emp-tool emp-zk gmp ssl crypto ntl gmp)

add_executable(test main.cpp polynomial.cpp clause.cpp clauseRAM.cpp)
target_link_libraries(test PUBLIC emp-tool emp-zk gmp ssl crypto ntl gmp)
