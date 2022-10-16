#include "store.h"

namespace wss {

Desc& Store::Add(std::int32_t fd)
{
    auto* desc = Find(fd);
    if (desc != nullptr)
        return *desc;

    auto [it, inserted] = descriptions_.emplace(fd, Desc(default_buffer_size_));
    assert(inserted);
    return it->second;
}

Desc& Store::Add(std::int32_t fd, const char* data, std::size_t size)
{
    auto& desc = Add(fd);
    desc.buffer.reserve(desc.buffer.size() + size);

    memcpy(desc.buffer.head(), data, size);
    desc.buffer.move_head(size);

    return desc;
}

Desc* Store::Find(std::int32_t fd)
{
    auto it = descriptions_.find(fd);
    if (it != descriptions_.end())
        return &(it->second);
    return nullptr;
}

const Desc* Store::Find(std::int32_t fd) const
{
    auto it = descriptions_.find(fd);
    if (it != descriptions_.end())
        return &(it->second);
    return nullptr;
}

template<typename F>
void Store::ForEach(F&& f)
{
    for (auto& it : descriptions_)
    {
        f(it.second);
    }
}

void Store::Remove(std::int32_t fd)
{
    descriptions_.erase(fd);
}

}