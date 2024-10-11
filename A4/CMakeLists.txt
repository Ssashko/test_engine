cmake_minimum_required(VERSION 3.24)
project(A4)
add_library (A4 INTERFACE)
add_executable (A4_mt_stability_stress_testing "mt_stability_stress_testing.cpp" "bullet_manager.cpp" "wall_manager.cpp")
add_executable (A4_performance_stress_testing_1 "performance_stress_testing_1.cpp" "bullet_manager.cpp" "wall_manager.cpp")
add_executable (A4_performance_stress_testing_2 "performance_stress_testing_2.cpp" "bullet_manager.cpp" "wall_manager.cpp")

target_link_libraries(A4 INTERFACE Engine)
target_compile_features(A4 INTERFACE cxx_std_20)

target_include_directories(A4 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(BOOST_INCLUDE_LIBRARIES system thread lockfree)
set(BOOST_ENABLE_CMAKE ON)

include(FetchContent)
FetchContent_Declare(
  Boost
  GIT_REPOSITORY https://github.com/boostorg/boost.git
  GIT_TAG boost-1.82.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Boost)

set(Boost_USE_MULTITHREADED ON)
target_link_libraries(A4 INTERFACE Boost::system Boost::thread Boost::lockfree)

target_link_libraries(A4_mt_stability_stress_testing PRIVATE A4)
target_link_libraries(A4_performance_stress_testing_1 PRIVATE A4)
target_link_libraries(A4_performance_stress_testing_2 PRIVATE A4)


get_target_property(EXECUTABLE_DIR A4_mt_stability_stress_testing RUNTIME_OUTPUT_DIRECTORY)

if(NOT EXECUTABLE_DIR)
    set(EXECUTABLE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
endif()

set(FONT_PATH_DESTINATION "${EXECUTABLE_DIR}" CACHE STRING "Path to detination of the Arial.ttf")