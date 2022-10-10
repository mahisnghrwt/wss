add_library(client client.cpp)

target_link_libraries(client poller)

# add_executable(sample_server_app sample_app.cpp)

# target_link_libraries(sample_server_app server)

add_executable(sample_poll_tty sample_poll_tty.cpp)