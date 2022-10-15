#include "client.h"

int main()
{
    wss::Client client(10555);
    if (!client.is_ok())
    {
        printf("coulnd't connect to the server\n");
        return 0;
    }
    
    client.Run();

    return 0;
}