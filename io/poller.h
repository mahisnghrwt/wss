#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cstdint>
#include <functional>
#include <poll.h>
#include <queue>

namespace wss {

class Poller
{
public:
    enum class Event : std::uint8_t
    {
        READ,
        WRITE,
        READ_WRITE
    };

    std::function<void(int fd)> on_read;
    std::function<void(int fd)> on_write;
    std::function<void(int fd, short flags)> on_hangup;
    std::function<void(int fd, short flags)> on_unexpected_event;
    std::function<void()> on_timeout;

    Poller(std::size_t nfds)
        : size_(0)
        , capacity_(nfds)
        , timeout_(-1)
    {
        fds_ = new pollfd[nfds];
    }

    ~Poller()
    {
        delete[] fds_;
    }

    bool AddFd(int fd, Event event);
    bool RemoveFd(int fd);
    void Run();

    void set_timeout(int32_t timeout) { timeout_ = timeout; }

private:
    const short unexpected_events = POLLPRI | POLLERR | POLLNVAL;
    const short hangup_events = POLLRDHUP | POLLHUP;

    pollfd* fds_;
    std::size_t size_;
    std::size_t capacity_;
    std::queue<std::size_t> reusable_fd_spots_;
    std::int32_t timeout_;
};

}