#include "poller.h"
#include <cassert>

namespace wss {
bool Poller::AddFd(int fd, Event event)
{
    if (size_ == capacity_ && reusable_fd_spots_.empty())
        return false;
    
    short events = 0;
    if (event == Event::READ || event == Event::READ_WRITE)
    {
        events |= POLLIN;
    }
    if (event == Event::WRITE || event == Event::READ_WRITE)
    {
        events |= POLLOUT;
    }
    events |= POLLRDHUP;

    std::size_t index = size_;
    if (!reusable_fd_spots_.empty())
    {
        index = reusable_fd_spots_.front();
        reusable_fd_spots_.pop();
    }
    else
    {
        ++size_;
    }

    auto* pfd = (fds_ + index);
    pfd->fd = fd;
    pfd->events = events;

    return true;
}

bool Poller::RemoveFd(int fd)
{
    if (size_ == 0)
        return false;
    for (std::size_t i = 0; i < size_; i++)
    {
        auto* pfd = (fds_ + i);
        if (pfd->fd == fd)
        {
            pfd->events = 0;
            pfd->revents = 0;
            pfd->fd = -1;
            reusable_fd_spots_.push(i);
            return true;
        }
    }
    return false;
}

void Poller::Run()
{
    auto events = poll(fds_, size_, -1);
    if (events == -1)
    {
        perror("poll returned");
        return;
    }

    while(true)
    {
        for (int i = 0; i < size_ && events > 0; i++)
        {
            auto* pfd = (fds_ + i);
            if (pfd->revents != 0)
            {
                assert(pfd->fd >= 0);
                if (pfd->revents & unexpected_events)
                {
                    if (on_unexpected_event)
                        on_unexpected_event(pfd->fd, pfd->revents);
                }
                if (pfd->revents & hangup_events)
                {
                    if (on_hangup)
                        on_hangup(pfd->fd, pfd->revents);
                }
                if (pfd->revents & POLLIN)
                {
                    if (on_read)
                        on_read(pfd->fd);
                }
                if (pfd->revents & POLLOUT)
                {
                    if (on_write)
                        on_write(pfd->fd);
                }

                pfd->revents = 0;
                --events;
            }
        }
    }
}
}