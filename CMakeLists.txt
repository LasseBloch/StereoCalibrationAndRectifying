cmake_minimum_required(VERSION 3.13)
project(StereoCalibrationAndRectifying)

set(CMAKE_CXX_STANDARD 17)

add_executable(StereoCalibrationAndRectifying main.cpp)

# Find and link opencv
find_package(OpenCV REQUIRED)
target_link_libraries(StereoCalibrationAndRectifying ${OpenCV_LIBS})

#Find threads to make raspberry happy
find_package(Threads)
target_link_libraries(StereoCalibrationAndRectifying ${CMAKE_THREAD_LIBS_INIT})