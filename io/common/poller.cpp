#include "poller.h"
#include "fd_desc.h"
#include "common/utils.h"
#include <cassert>
#include <sys/socket.h>
#include <unistd.h>

#ifdef DEBUG
#define CHECK_FOR_EVENT(events, event_to_check, out_events) \
    { \
        if (events & event_to_check) \
        { \
            out_events = out_events + " " + #event_to_check; \
        } \
    }

#define LOG_EVENTS(fd, events) \
    { \
        std::string events_received; \
        CHECK_FOR_EVENT(events, POLLIN, events_received); \
        CHECK_FOR_EVENT(events, POLLOUT, events_received); \
        CHECK_FOR_EVENT(events, POLLHUP, events_received); \
        CHECK_FOR_EVENT(events, POLLRDHUP, events_received); \
        CHECK_FOR_EVENT(events, POLLNVAL, events_received); \
        CHECK_FOR_EVENT(events, POLLERR, events_received); \
        CHECK_FOR_EVENT(events, POLLPRI, events_received); \
        LOG_DEBUG("fd(%d) following events received: (%s)\n", fd, events_received.c_str()); \
    }
#else
#define LOG_EVENTS(...)
#endif

namespace wss {
void Poller::AddFd(int fd, uint8_t event)
{
    if (!safe_to_update_fd_)
    {
        pending_tasks_.emplace_back([this, fd, event]()
        { 
            AddFd(fd, event);
        });
        return;
    }

    short flags = POLLRDHUP;
    if (event & Event::READ)
        flags |= POLLIN;
    if (event & Event::WRITE)
        flags |= POLLOUT;

    fds_.emplace_back(pollfd{fd, flags, 0});

    LOG_DEBUG("fd(%d) polling for event(%d)\n", fd, event);
}

void Poller::RemoveFd(int fd)
{
    if (!safe_to_update_fd_)
    {
        pending_tasks_.emplace_back([this, fd]()
        {
            RemoveFd(fd);
        });
        return;
    }

    for (auto fds_it = fds_.begin(); fds_it != fds_.end(); ++fds_it)
    {
        if (fds_it->fd == fd)
        {
            fds_.erase(fds_it);
            break;
        }
    }
    
    LOG_DEBUG("fd(%d) removed\n", fd);
}

void Poller::UpdateFd(int fd, uint8_t event)
{
    if (!safe_to_update_fd_)
    {
        pending_tasks_.emplace_back([this, fd, event]()
        {
            UpdateFd(fd, event);
        });
        return;
    }

    for (auto fds_it = fds_.begin(); fds_it != fds_.end(); ++fds_it)
    {
        if (fds_it->fd == fd)
        {
            fds_it->events = 0;
            if (READ & event)
                fds_it->events |= POLLIN;
            if (WRITE & event)
                fds_it->events |= POLLOUT;
            break;
        }
    }
    
    LOG_DEBUG("fd(%d) events updated(%d)\n", fd, event);
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

        LOG_DEBUG("(%d) fd received events\n", events);

        safe_to_update_fd_ = false;

        for (auto it = fds_.begin(); it != fds_.end() && events > 0; ++it)
        {
            auto& pfd = *it;
            if (pfd.revents != 0)
            {
                LOG_EVENTS(pfd.fd, pfd.revents);

                if (pfd.revents & POLLNVAL)
                {
                    OnInvalidFd(pfd.fd);
                }
                else if (pfd.revents & POLLIN || pfd.revents & POLLOUT)
                {
                    if (pfd.revents & POLLIN)
                        OnRead(pfd.fd);
                    if (pfd.revents & POLLOUT)
                        OnWrite(pfd.fd);
                } 
                if (pfd.revents & POLLRDHUP)
                {
                    OnConnectionAborted(pfd.fd);
                }                

                pfd.revents = 0;
                --events;
            }
        }

        safe_to_update_fd_ = true;

        OnAllEventsRead();
        OnAllEventsReadImpl();
    }
}

void Poller::OnAllEventsRead()
{
    LOG_DEBUG("executing (%ld) pending tasks\n", pending_tasks_.size());

    assert(safe_to_update_fd_);

    for (auto& task : pending_tasks_)
    {
        task();
    }

    pending_tasks_.clear();
}

}