#pragma once

#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <error.h>
#include <locale>
#include <stdlib.h>

template<typename StrT, typename... Ts>
void Panic(int err_num, StrT format_str, Ts&&... args)
{
    std::string form_str_err = std::string(format_str);
    form_str_err += " errno(%d)\n";
    fprintf(stderr, form_str_err.c_str(), args..., err_num);
    // exit(1);
}

template<typename StrT, typename... Ts>
void Panic(StrT&& format_str, Ts&&... args)
{
    auto err_num = errno;
    Panic(err_num, std::forward<StrT>(format_str), std::forward<Ts>(args)...);
}

template<typename StrT>
void Panic(StrT&& format_str)
{
    auto err_num = errno;
    std::string proxy_str(format_str);
    proxy_str += "%s";
    Panic(err_num, proxy_str, "");
}

namespace wss::utils {

template<typename T, typename StrT>
bool pperror(T rv, StrT&& msg)
{
    if (rv == -1) { perror(msg); return true; }
    return false;
}

template<typename T>
bool pperror(T rv)
{ return pperror(rv, ""); }

}

inline const char* get_local_time()
{
    static char str[9];
    memset(str, '\0', sizeof(str));
    std::time_t t = std::time(nullptr);

    if (std::strftime(str, sizeof(str), "%T", std::localtime(&t)) == 0)
        assert(false);

    return str;
}

#define LOG(...) \
    printf("%9s | %s | ", get_local_time(), __FUNCTION__); \
    printf(__VA_ARGS__);
