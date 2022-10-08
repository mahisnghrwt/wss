add_library(server server.cpp)

target_link_libraries(server poller)

add_executable(sample_server_app sample_app.cpp)

target_link_libraries(sample_server_app server)