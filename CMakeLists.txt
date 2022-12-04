cmake_minimum_required (VERSION 3.16.3)
project (wss)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set_property(GLOBAL PROPERTY CXX_STANDARD 17)

add_executable(server2_app server2_app.cpp server2.cpp)
add_executable(client2_app client2_app.cpp client2.cpp)