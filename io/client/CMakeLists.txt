add_library(client client.cpp)
target_link_libraries(client poller)

add_executable(simple_client simple_client.cpp)

add_executable(sample_client_app sample_client_app.cpp)
target_link_libraries(sample_client_app client)

add_executable(sample_poll_tty sample_poll_tty.cpp)