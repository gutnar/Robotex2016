cmake_minimum_required(VERSION 2.8.11)
project(Robotex2016)

# Source files to compile
add_executable(Robotex2016 src/main.cpp src/Calibrator.cpp src/Calibrator.h src/Detector.cpp src/Detector.h lib/ConvertUTF.c lib/snippets.cpp src/AI.cpp src/AI.h src/common.h src/Communicator.cpp src/Communicator.h)

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(Robotex2016 ${OpenCV_LIBS})

# libserialport
#target_link_libraries(Robotex2016 ${libserialport_LIBRARIES})
#include_directories(${libserialport_INCLUDE_DIRS})
target_link_libraries(Robotex2016 -L/usr/local/lib -lserialport)
include_directories(/usr/local/include)

# Lib
include_directories(lib)