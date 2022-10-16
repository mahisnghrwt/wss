#include "server.h"
#include <memory>
#include <signal.h>

std::unique_ptr<wss::Server> server = nullptr;

void signal_handler(int signal)
{
    printf("singal caught(%d)", signal);
    if (server != nullptr)
        server->ShutdownAll();
}

int main(int argc, char const* argv[])
{
    signal(SIGINT, signal_handler);

    server = std::make_unique<wss::Server>(10555, 3);
    
    server->Run();

    return 0;
}
