add_executable(client client.cpp)

add_library(poller STATIC poller.cpp)

add_subdirectory(server)