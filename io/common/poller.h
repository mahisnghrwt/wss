#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cstdint>
#include <functional>
#include <memory>
#include <poll.h>
#include <queue>
#include <vector>

namespace wss {

class Poller
{
public:
    enum Event : std::uint8_t
    {
        READ = 1,
        WRITE = 2
    };

    Poller() = delete;

    Poller()
        : timeout_(-1)
        , safe_to_update_fd_(true)
    {}

    virtual ~Poller() {}

    void AddFd(int fd, std::uint8_t event);
    void RemoveFd(int fd);
    void UpdateFd(int fd, uint8_t event);

    void Run();

    void set_timeout(int32_t timeout) { timeout_ = timeout; }

protected:
    virtual void OnRead(std::int32_t fd) = 0;
    virtual void OnWrite(std::int32_t fd) = 0;
    virtual void OnInvalidFd(std::int32_t fd) = 0;
    virtual void OnConnectionAborted(std::int32_t fd) = 0;

    virtual void OnTimeout() {};
    virtual void OnAllEventsReadImpl() {};

private:
    void OnAllEventsRead();

    std::vector<std::function<void()>> pending_tasks_;
    bool safe_to_update_fd_;
    std::int32_t timeout_;
    std::vector<pollfd> fds_;
};

}