#include <cassert>
#include <iostream>
#include "buffer.h"

void test_1()
{    
    // buffer full fill test
    // fill the buffer without getting sigabrt
    // buffer content must be same as source content

    char random_str[] = "abcdefghi";
    auto l = sizeof(random_str);
    Buffer buffer(l);

    auto* write_ptr = buffer.head();
    memcpy(write_ptr, random_str, l);
    buffer.move_head(l);

    assert(buffer.cap_available() == 0);
    assert(memcmp(buffer.data(), random_str, l) == 0);
}

bool check(char** strings, Buffer& buffer, std::size_t index, const std::size_t n, const std::size_t str_len)
{
    if (index == n)
        return true;

    auto* str = *(strings + index);
    assert(buffer.cap_available() >= str_len);
    auto* write_ptr = buffer.head();
    memcpy(write_ptr, str, str_len);
    buffer.move_head(str_len);

    char* data = (char*)buffer.data();
    for (std::size_t i = 0; i <= index; i++)
    {
        for (std::size_t j = 0; j < str_len; j++)
        {
            if (*data != *(*(strings + i) + j))
            {
                return false;
            }
            ++data;
        }
    }

    return check(strings, buffer, index + 1, n, str_len);
};

void test_2()
{
    char str_1[] = "6612256222";
    char str_2[] = "8382990589";
    char str_3[] = "6511638820";

    char** strings = new char*[3];
    *(strings) = str_1;
    *(strings + 1) = str_2;
    *(strings + 2) = str_3;

    auto str_len = sizeof(str_1);
    assert(str_len == sizeof(str_2) & str_len == sizeof(str_3));

    auto array_size = 3;

    Buffer buffer(array_size * str_len);

    assert(check(strings, buffer, 0, array_size, str_len));
}

int main()
{
    test_1();
    test_2();
    return 0;
}