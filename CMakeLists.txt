cmake_minimum_required(VERSION 2.8.11)
project(Robotex2016)

# Source files to compile
add_executable(Robotex2016 src/main.cpp src/Calibrator.cpp src/Calibrator.h src/Detector.cpp src/Detector.h lib/ConvertUTF.c lib/snippets.cpp)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(Robotex2016 ${OpenCV_LIBS})

include_directories(lib)