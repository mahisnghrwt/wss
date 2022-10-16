#pragma once

#include <cstdio>
#include <cstring>
#include <cassert>

class Buffer
{
public:
    Buffer(std::size_t capacity)
        : capacity_(capacity)
    {
        storage_ = new char[capacity_];
        Clear();
    }

    ~Buffer()
    { delete[] storage_; }

    void move_head(std::size_t bytes_written)
    {
        assert(bytes_written >= 0);
        assert(cap_available() >= bytes_written);
        offset_ += bytes_written;
    }

    const char* data() const { return storage_; }

    char* head() { return storage_ + offset_; }
    const char* head() const { return storage_ + offset_; }

    std::size_t size() const { return offset_; }
    std::size_t cap_available() const { return capacity_ - offset_; }
    std::size_t capacity() const { return capacity_; }

    void Clear()
    {
        memset(storage_, 0, capacity_);
        offset_ = 0;
    }

private:
    char* storage_;
    std::size_t offset_ = 0;
    std::size_t capacity_ = 0;
};