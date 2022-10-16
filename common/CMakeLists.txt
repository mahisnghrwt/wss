add_executable(buffer_tests buffer_tests.cpp)
add_executable(log_tests log_tests.cpp)

target_compile_definitions(log_tests PUBLIC DEBUG)