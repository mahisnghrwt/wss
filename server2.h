#include "common.h"
#include "utils.h"
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <poll.h>
#include <algorithm>
#include <cassert>
#include <functional>
#include <netinet/ip.h>
#include <vector>

namespace wss {
class Server2
{
public:
    Server2(Port port);
    void Run();
    void Shutdown();

private:
    void AddFd(Fd fd, Event event);
    void OnNewConnection(Fd fd); // TODO: rename to OnClientConnected
    void OnData(Fd fd);
    void OnClientDisconnected(Fd fd);

    const Port port_;
    Fd server_fd_;
    FdList fd_list_;
    sockaddr_in address_;
    bool is_ok_ = true;
#define READ_BUFFER_SIZE 1024
};
}