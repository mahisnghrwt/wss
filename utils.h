#pragma once

#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <error.h>
#include <locale>
#include <stdlib.h>

namespace wss {

template<typename T, typename StrT>
bool is_error(T rv, StrT&& msg)
{
    if (rv == -1) { perror(msg); return true; }
    return false;
}

template<typename T>
bool is_error(T rv)
{ return is_error(rv, ""); }

inline const char* get_local_time()
{
    static char str[9];
    memset(str, '\0', sizeof(str));
    std::time_t t = std::time(nullptr);

    if (std::strftime(str, sizeof(str), "%T", std::localtime(&t)) == 0)
        assert(false);

    return str;
}

#define LOG(...) { \
    printf("%9s [%s] ", get_local_time(), __PRETTY_FUNCTION__); \
    printf(__VA_ARGS__); \
    }

}