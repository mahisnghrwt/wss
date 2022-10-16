cmake_minimum_required (VERSION 3.16.3)
project (wss)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CXX_STANDARD 17)

add_subdirectory(common)
include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(io)

add_custom_target(
    run_buffer_tests
    ALL
    COMMAND
        buffer_tests
    COMMENT
        "Running tests..."
)