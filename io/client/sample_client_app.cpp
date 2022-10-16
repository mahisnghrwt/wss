#include "client.h"
#include "common/utils.h"
#include <signal.h>
#include <string>

wss::Client* client = nullptr;

void signal_handler(int signall)
{
    LOG_DEBUG("signal handler triggered\n");
    if (client != nullptr)
        client->Shutdown();
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        LOG("expected PORT as a command line argument\n");
        return 1;
    }

    std::int32_t port = std::stoi(*(argv + 1));
    client = new wss::Client(port);

    signal(SIGINT, signal_handler);

    if (!client->is_ok())
    {
        LOG("coulnd't connect to the server\n");
        return 0;
    }
    
    client->Run();

    return 0;
}