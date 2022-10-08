add_executable(client client.cpp)

add_library(poller
            STATIC
            poller.cpp
            fd_desc.cpp)

add_subdirectory(server)