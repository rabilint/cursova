cmake_minimum_required(VERSION 4.0)
project(Receiver)

set(CMAKE_CXX_STANDARD 20)
include_directories(vendor/serial/include)
file(GLOB SERIAL_SOURCES "vendor/serial/src/*.cc" "vendor/serial/src/impl/*.cc")
file(GLOB SQLITE_SOURCES "vendor/sqlite3/*.c")

include_directories(vendor/sqlite)
#set(SQLITE_SOURCES vendor/sqlite/sqlite3.c)

add_executable(Receiver main.cpp ${SERIAL_SOURCES}
        SerialCommunicator.cpp
        SerialCommunicator.h
        DatabaseManager.cpp
        DatabaseManager.h
        ${SQLITE_SOURCES}
)

if (UNIX AND NOT APPLE)
    target_link_libraries(Receiver PRIVATE pthread)
endif()

find_library(RT_LIBRARY rt)
if(RT_LIBRARY)
    target_link_libraries(Receiver PRIVATE ${RT_LIBRARY})
endif()
