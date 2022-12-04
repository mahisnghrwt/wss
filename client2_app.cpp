#include "client2.h"
#include <memory>
#include <signal.h>
#include <string>

std::unique_ptr<wss::Client2> client = nullptr;

void signal_handler(int signal)
{
    printf("singal caught(%d)\n", signal);
    if (client != nullptr)
        client->Shutdown();
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
    client = std::make_unique<wss::Client2>(port);
    client->Run();

    return 0;
}