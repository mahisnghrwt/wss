cmake_minimum_required (VERSION 3.16.3)
project (wss)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set_property(GLOBAL PROPERTY CXX_STANDARD 17)

add_executable(buffer_tests buffer_tests.cpp)
add_executable(log_tests log_tests.cpp)

add_custom_target(
    run_buffer_tests
    ALL
    COMMAND
        buffer_tests
    COMMAND
        log_tests
    COMMENT
        "Running tests..."
    )

add_subdirectory(io)