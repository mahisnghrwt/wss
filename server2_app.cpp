#include "server2.h"
#include <memory>
#include <signal.h>
#include <string>

std::unique_ptr<wss::Server2> server = nullptr;

void signal_handler(int signal)
{
    printf("singal caught(%d)\n", signal);
    if (server != nullptr)
        server->Shutdown();
    exit(0);    
}

int main(int argc, char const* argv[])
{
    signal(SIGINT, signal_handler);

    if (argc != 2)
    {
        printf("must pass port as a command line argument\n");
        return 0;
    }

    const auto port = std::stoi(*(argv + 1));
    printf("port: %d\n", port);
    server = std::make_unique<wss::Server2>(port);
    server->Run();

    return 0;
}
