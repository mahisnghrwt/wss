#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "io/common/poller.h"
#include "io/common/store.h"
#include "common/buffer.h"
#include <cstdint>
#include <poll.h>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unordered_map>

namespace wss {

class Server : public Poller
{
public:
    static const constexpr std::size_t BUFFER_SIZE = 1024;

    Server(int port, std::size_t connection_backlog)
        : Poller()
        , port_(port)
        , connection_backlog_(connection_backlog)
        , server_fd_(-1)
        , buffer_(BUFFER_SIZE)
        , is_ok_(true)
    {
        init();
    }

    ~Server()
    {
        ShutdownAll();
    }

    bool is_ok() const { return is_ok_; }

    void Run();

    void ShutdownAll();

private:
    void init();

    void OnRead(std::int32_t fd) override;

    void OnNewConnection();

    void OnClientData(std::int32_t fd);

    void OnWrite(std::int32_t fd) override;

    void OnInvalidFd(std::int32_t fd) override;

    void OnConnectionAborted(std::int32_t fd) override;

    void Shutdown(std::int32_t fd);

    const std::int32_t port_;
    const std::size_t connection_backlog_;
    std::int32_t server_fd_;
    std::vector<std::int32_t> client_fds_;
    Buffer buffer_;
    bool is_ok_;
    sockaddr_in address_;
    Store store_;
};

}