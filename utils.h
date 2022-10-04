#pragma once

#include <cstdio>
#include <string>
#include <error.h>
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