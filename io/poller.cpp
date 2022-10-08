#include "poller.h"
#include "fd_desc.h"
#include "../utils.h"
#include <cassert>
#include <sys/socket.h>
#include <unistd.h>

namespace wss {
bool Poller::AddFd(int fd, Event event)
{
    if (fds_.size() == capacity_)
        return false;
    
    short flags = 0;
    if (event & Event::READ)
    {
        flags |= POLLIN;
    }
    if (event & Event::WRITE)
    {
        flags |= POLLOUT;
    }
    flags |= POLLRDHUP;

    fds_.emplace_back(pollfd{ fd, flags, 0 });

    return true;
}

bool Poller::RemoveFd(int fd)
{
    for (auto fds_it = fds_.begin(); fds_it != fds_.end(); ++fds_it)
    {
        if (fds_it->fd == fd)
        {
            fds_.erase(fds_it);
            return true;
        }
    }
    return false;
}

void Poller::Run()
{
    while(true)
    {
        auto events = poll(fds_.data(), fds_.size(), timeout_);
        if (utils::pperror(events))
        {
            return;
        }
        else if (events == 0)
        {
            OnTimeout();
            return;
        }

        for (auto it = fds_.begin(); it != fds_.end() && events > 0; ++it)
        {
            auto& pfd = *it;
            if (pfd.revents != 0)
            {
                if (pfd.revents & POLLERR || pfd.events & POLLPRI)
                {
                    OnPollerr(pfd.fd);
                }
                if (pfd.revents & POLLHUP || pfd.events & POLLRDHUP)
                {
                    OnHangup(pfd.fd);
                }
                if (pfd.revents & POLLIN)
                {
                    OnRead(pfd.fd);
                }
                if (pfd.revents & POLLOUT)
                {
                    OnWrite(pfd.fd);
                }
                if (pfd.revents & POLLNVAL)
                {
                    OnPollnval(pfd.fd);
                }

                pfd.revents = 0;
                --events;
            }
        }
    }
}

void Poller::OnHangup(std::int32_t fd)
{
    auto* desc = fd_store_->Get(fd);
    desc->state = PollfdDesc::SHUTDOWN;

    if (desc != nullptr && desc->eof_received)
    {
        OnEOF(fd);
    }
    else if (desc != nullptr && utils::pperror(shutdown(fd, SHUT_RDWR)))
    {}
    else
    {
        desc->state = PollfdDesc::CLOSED;
        close(fd);
        fd_store_->Remove(fd);
    }
}

void Poller::OnEOF(std::int32_t fd)
{
    auto* desc = fd_store_->Get(fd);
    assert(desc != nullptr);
    // log and don't assert
    if (!desc->eof_received)
    {
        printf("! (%d) eof not received\n", fd);
    }
    if (desc->state == PollfdDesc::CLOSED)
    {
        printf("! (%d) already in CLOSED state\n", fd);
    }
    if (desc->state != PollfdDesc::SHUTDOWN)
    {
        utils::pperror(shutdown(fd, SHUT_RDWR));
    }

    utils::pperror(close(fd));
  
    RemoveFd(fd);

    fd_store_->Remove(fd);
}

void Poller::OnPollnval(std::int32_t fd)
{
    auto* desc = fd_store_->Get(fd);
    utils::pperror(close(fd));
    RemoveFd(fd);
    fd_store_->Remove(fd);
}

void Poller::OnPollerr(std::int32_t fd)
{
    printf("! (%d) pollerr received\n", fd);
}

}