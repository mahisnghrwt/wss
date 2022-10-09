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
    fd_store_->Add(fd, PollfdDesc{PollfdDesc::OPEN, false});

    LOG("fd(%d) polling for event(%d)\n", fd, event);
    
    return true;
}

bool Poller::RemoveFd(int fd)
{
    if (unsafe_to_remove_fd_)
    {
        fd_pending_removal_.emplace_back(fd);
        LOG("fd(%d) pending removal\n", fd);
        return false;
    }
    for (auto fds_it = fds_.begin(); fds_it != fds_.end(); ++fds_it)
    {
        if (fds_it->fd == fd)
        {
            fds_.erase(fds_it);
            break;
        }
    }
    fd_store_->Remove(fd);
    LOG("fd(%d) removed\n", fd);
    return true;
}

void Poller::Run()
{
    while(true)
    {
        auto events = poll(fds_.data(), fds_.size(), timeout_);
        if (utils::pperror(events))
            return;
            
        else if (events == 0)
        {
            OnTimeout();
            return;
        }

        LOG("%d events received\n", events);

        unsafe_to_remove_fd_ = true;
        for (auto it = fds_.begin(); it != fds_.end() && events > 0; ++it)
        {
            auto& pfd = *it;
            if (pfd.revents != 0)
            {
                if (pfd.revents & POLLIN)
                {
                    OnRead(pfd.fd);
                }
                else
                {
                    if (pfd.revents & POLLERR || pfd.events & POLLPRI)
                    {
                        OnPollerr(pfd.fd);
                    }
                    if (pfd.revents & POLLHUP || pfd.events & POLLRDHUP)
                    {
                        OnHangup(pfd.fd);
                    }
                    // if (pfd.revents & POLLOUT)
                    // {
                    //     OnWrite(pfd.fd);
                    // }
                    if (pfd.revents & POLLNVAL)
                    {
                        OnPollnval(pfd.fd);
                    }
                }                

                pfd.revents = 0;
                --events;
            }
        }

        unsafe_to_remove_fd_ = false;
        while(!fd_pending_removal_.empty())
        {
            RemoveFd(fd_pending_removal_.back());
            fd_pending_removal_.pop_back();
        }
    }
}

void Poller::OnHangup(std::int32_t fd)
{
    LOG("fd(%d) hungup\n", fd);

    auto* desc = fd_store_->Get(fd);

    if (desc != nullptr && desc->eof_received)
    {
        OnEOF(fd);
        desc->state = PollfdDesc::SHUTDOWN;
    }
    else if (desc != nullptr && utils::pperror(shutdown(fd, SHUT_RDWR)))
    {
        desc->state = PollfdDesc::SHUTDOWN;
    }
    else
    {
        desc->state = PollfdDesc::CLOSED;
        close(fd);
        RemoveFd(fd);
    }
}

void Poller::OnEOF(std::int32_t fd)
{
    LOG("fd(%d) EOF received\n", fd);

    auto* desc = fd_store_->Get(fd);
    
    if (desc != nullptr)
    {
        if (!desc->eof_received)
            LOG("fd(%d) EOF NOT received\n", fd);
        if (desc->state == PollfdDesc::CLOSED)
            LOG("fd(%d) already in closed state\n", fd);
        if (desc->state != PollfdDesc::SHUTDOWN)
            utils::pperror(shutdown(fd, SHUT_RDWR));
    }  

    LOG("fd(%d) closing connection\n", fd);
    utils::pperror(close(fd));
    RemoveFd(fd);
}

void Poller::OnPollnval(std::int32_t fd)
{
    LOG("fd(%d) POLLNVAL received, closing connection\n", fd);
    utils::pperror(close(fd));
    RemoveFd(fd);
}

void Poller::OnPollerr(std::int32_t fd)
{
    LOG("fd(%d) POLLERR received, ignoring\n", fd);
}

}