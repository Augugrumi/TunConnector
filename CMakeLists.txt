cmake_minimum_required(VERSION 3.5.1)
project(TunConnector)

set(CMAKE_CXX_STANDARD 11)

find_package(Threads REQUIRED)
find_package(Boost COMPONENTS log REQUIRED)

add_executable(TunConnector main.cpp src/TunConnector.cpp src/TunConnector.h)

target_link_libraries(TunConnector ${CMAKE_THREAD_LIBS_INIT} ${Boost_LOG_LIBRARY} )
add_definitions(-DBOOST_LOG_DYN_LINK)