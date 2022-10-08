#pragma once

#include <unordered_map>

namespace wss {

struct PollfdDesc
{
    enum State : std::uint8_t
    {
        OPEN,
        SHUTDOWN,
        CLOSED
    };

    State state;
    bool eof_received;
};

class PollfdStore
{
public:
    const PollfdDesc* Add(const std::int32_t fd, const PollfdDesc& desc);

    PollfdDesc* Get(const std::int32_t fd);
    const PollfdDesc* Get(const std::int32_t fd) const;

    void Remove(const std::int32_t fd);

private:
    std::unordered_map<std::int32_t, PollfdDesc> descriptions_;
};

}