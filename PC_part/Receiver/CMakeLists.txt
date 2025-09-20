cmake_minimum_required(VERSION 4.0)
project(Receiver)

set(CMAKE_CXX_STANDARD 20)
include_directories(vendor/serial/include)
file(GLOB SERIAL_SOURCES "vendor/serial/src/*.cc" "vendor/serial/src/impl/*.cc")

add_executable(Receiver main.cpp ${SERIAL_SOURCES}
        SerialCommunicator.cpp
        SerialCommunicator.h
        DatabaseManager.cpp
        DatabaseManager.h)

if (UNIX AND NOT APPLE)
    target_link_libraries(Receiver PRIVATE pthread)
endif()

find_library(RT_LIBRARY rt)
if(RT_LIBRARY)
    target_link_libraries(Receiver PRIVATE ${RT_LIBRARY})
endif()
