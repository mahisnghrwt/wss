#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cstdint>
#include <functional>
#include <poll.h>
#include <queue>
#include <vector>

namespace wss {

class Poller
{
public:
    enum Event : std::uint8_t
    {
        READ = 0b1,
        WRITE = 0b10,
        READ_WRITE = 0b11
    };

    std::function<void(int fd)> on_read;
    std::function<void(int fd)> on_write;
    std::function<void(int fd, short flags)> on_hangup;
    std::function<void(int fd, short flags)> on_unexpected_event;
    std::function<void()> on_timeout;

    Poller(std::size_t nfds)
        : capacity_(nfds)
        , timeout_(-1)
    {}

    // close all fd in fds_

    bool AddFd(int fd, Event event);
    bool RemoveFd(int fd);
    void Run();

    void set_timeout(int32_t timeout) { timeout_ = timeout; }

private:
    const short unexpected_events = POLLPRI | POLLERR | POLLNVAL;
    const short hangup_events = POLLRDHUP | POLLHUP;

    std::vector<pollfd> fds_;
    std::size_t capacity_;
    std::int32_t timeout_;
};

}