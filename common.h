#pragma once

#include "utils.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <vector>
#include <poll.h>

namespace wss {

using Port = std::int32_t;
using Fd = std::int32_t;

enum Event : std::uint8_t
{
    READ = 0b1,
    WRITE = 0b10,
    READ_WRITE = 0b11
};

class FdList
{
public:
    void Add(Fd fd, short flags)
    {
        auto task = [&]() {
            assert(safe_);
            fds_.emplace_back(pollfd{fd, flags, 0});
        };

        if (safe_)
        {
            task();
        }
        else
        {
            tasks_.emplace_back(std::move(task));
        }
    }

    void Remove(Fd fd)
    {
        auto task = [this, fd]() {
            assert(safe_);
            std::remove_if(fds_.begin(), fds_.end(), [fd](const auto& f) {
                if (f.fd == fd)
                    return true;
                return false;
            });
        };

        if (safe_)
        {
            task();
        }
        else
        {
            tasks_.emplace_back(std::move(task));
        }
    }

    void Update(Fd fd, short flags)
    {
        auto task = [&]() {
            assert(safe_);
            auto it = std::find_if(fds_.begin(), fds_.end(), [fd](const auto& f) {
                if (f.fd == fd)
                    return true;
                return false;
            });

            if (it != fds_.end())
            {
                it->events = flags;
            }
            else
            {
                assert(false);
            }
        };

        if (safe_)
        {
            task();
        }
        else
        {
            tasks_.emplace_back(std::move(task));
        }
    }

    void Lock() { safe_ = false; }
    void Unlock()
    {
        safe_ = true;
        for (auto& task : tasks_)
        {
            task();
        }
        tasks_.clear();
    }

    std::vector<pollfd>& fds() { return fds_; }

private:
    std::vector<pollfd> fds_;
    bool safe_ = true;
    std::vector<std::function<void()>> tasks_;
};

inline void LogPollEvents(const pollfd& p)
{
    auto str = std::to_string(p.fd);
#define CHECK_EVENT(event) if (p.revents & event) { str = str + " " + #event; }
    CHECK_EVENT(POLLIN);
    CHECK_EVENT(POLLPRI);
    CHECK_EVENT(POLLOUT);
    CHECK_EVENT(POLLRDHUP);
    CHECK_EVENT(POLLERR);
    CHECK_EVENT(POLLHUP);
    CHECK_EVENT(POLLNVAL);
    LOG("%s", str.c_str());
#undef CHECK_EVENT
}

}