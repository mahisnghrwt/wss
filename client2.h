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
class Client2
{
public:
    Client2(Port port);
    void Run();
    void Shutdown();

private:
    void AddFd(Fd fd, Event event);
    void OnData(Fd fd);
    void OnWriteData(Fd fd);

    const Port port_;
    Fd fd_;
    FdList fd_list_;
    sockaddr_in address_;
    bool is_ok_ = true;

#define READ_BUFFER_SIZE 1024
#define WRITE_BUFFER_SIZE 128
    char write_buffer_[WRITE_BUFFER_SIZE];
    char read_buffer_[READ_BUFFER_SIZE];
};
}