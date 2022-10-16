#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "fd_desc.h"
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
        READ = 0b1,
        WRITE = 0b10,
        READ_WRITE = 0b11
    };

    Poller(std::size_t nfds)
        : capacity_(nfds)
        , timeout_(-1)
        , unsafe_to_remove_fd_(false)
    {
        fd_store_ = std::make_unique<PollfdStore>();
    }

    virtual ~Poller() {}

    bool AddFd(int fd, Event event);
    bool RemoveFd(int fd);
    void Run();

    void set_timeout(int32_t timeout) { timeout_ = timeout; }

protected:
    virtual void OnRead(std::int32_t fd) = 0;
    virtual void OnWrite(std::int32_t fd) = 0;
    
    // if EOF alredy received, call OnEOF.
    // else, try shutting down the connection,
    // if fails, close the connection and stop polling the fd
    virtual void OnHangup(std::int32_t fd);
    
    // shutdown if not already. close the connection and stop polling the fd
    virtual void OnEOF(std::int32_t fd);

    // log and stop polling the fd
    virtual void OnPollnval(std::int32_t fd);
    
    // log and ignore
    virtual void OnPollerr(std::int32_t fd);

    virtual void OnTimeout() {};

protected:
    std::unique_ptr<PollfdStore> fd_store_;
    std::vector<pollfd> fds_;

private:
    std::size_t capacity_;
    std::int32_t timeout_;
    bool unsafe_to_remove_fd_;
    std::vector<std::int32_t> fd_pending_removal_;
};

}