#include "server.h"
#include "common/utils.h"
#include <memory>
#include <signal.h>

std::unique_ptr<wss::Server> server = nullptr;

void signal_handler(int signal)
{
    LOG("singal caught(%d)\n", signal);
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
