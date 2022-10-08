#include "fd_desc.h"
#include <cassert>

namespace wss {

const PollfdDesc* PollfdStore::Add(const std::int32_t fd, const PollfdDesc& desc)
{
    auto [it, inserted] = descriptions_.emplace(fd, desc);
    assert(inserted);
    return &it->second;
}

PollfdDesc* PollfdStore::Get(const std::int32_t fd)
{
    auto it = descriptions_.find(fd);
    if (it == descriptions_.end())
        return nullptr;
    return &(it->second);
}

const PollfdDesc* PollfdStore::Get(const std::int32_t fd) const
{
    auto it = descriptions_.find(fd);
    if (it == descriptions_.end())
        return nullptr;
    return &(it->second);
}

void PollfdStore::Remove(const std::int32_t fd)
{
    descriptions_.erase(fd);
}

}