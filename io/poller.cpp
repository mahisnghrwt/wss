#include "poller.h"
#include <cassert>

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

    fds_.emplace_back(pollfd{fd, flags, 0});

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
        if (events == -1)
        {
            perror("poll returned");
            return;
        }
        else if (events == 0)
        {
            if (on_timeout)
                on_timeout();
            return;
        }

        for (auto it = fds_.begin(); it != fds_.end() && events > 0; ++it)
        {
            auto& pfd = *it;
            if (pfd.revents != 0)
            {
                assert(pfd.fd >= 0);
                if (pfd.revents & unexpected_events)
                {
                    if (on_unexpected_event)
                        on_unexpected_event(pfd.fd, pfd.revents);
                }
                if (pfd.revents & hangup_events)
                {
                    if (on_hangup)
                        on_hangup(pfd.fd, pfd.revents);
                }
                if (pfd.revents & POLLIN)
                {
                    if (on_read)
                        on_read(pfd.fd);
                }
                if (pfd.revents & POLLOUT)
                {
                    if (on_write)
                        on_write(pfd.fd);
                }

                pfd.revents = 0;
                --events;
            }
        }
    }
}
}