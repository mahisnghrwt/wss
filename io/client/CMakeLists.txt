add_library(client client.cpp)
target_link_libraries(client poller)

add_executable(sample_client_app sample_client_app.cpp)
target_link_libraries(sample_client_app client)