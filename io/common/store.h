#include "common/buffer.h"
#include <unordered_map>

namespace wss {

struct Desc
{
    Desc(std::size_t buffer_cap)
        : buffer(buffer_cap)
    {}

    Buffer buffer;
};

class Store
{
public:
    Desc& Add(std::int32_t fd);
    Desc& Add(std::int32_t fd, const char* data, std::size_t size);

    Desc* Find(std::int32_t fd);
    const Desc* Find(std::int32_t fd) const;

    template<typename F>
    void ForEach(F&& f);

    void Remove(std::int32_t fd);   

private:
    const std::size_t default_buffer_size_ = 1024;
    std::unordered_map<std::int32_t, Desc> descriptions_;
};

}