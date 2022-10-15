#include "client.h"
#include "../../utils.h"
#include <signal.h>

wss::Client client(10555);

void signal_handler(int signall)
{
    LOG_DEBUG("signal handler triggered\n");
    client.Shutdown();
}

int main()
{
    signal(SIGINT, signal_handler);

    if (!client.is_ok())
    {
        printf("coulnd't connect to the server\n");
        return 0;
    }
    
    client.Run();

    return 0;
}