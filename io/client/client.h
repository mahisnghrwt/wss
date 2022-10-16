/*
 * Client - reads user input
 * reads and writes on the socket
 */

#pragma once

#include "io/common/poller.h"
#include "common/buffer.h"

namespace wss {

class Client : public Poller
{
public:
    const std::size_t BUFFER_SIZE = 1024;

    Client(std::int32_t port);

    void Run() { Poller::Run(); }

    void Shutdown();

    bool is_ok() const { return is_ok_; }

private:
    void Init();
    void OnRead(std::int32_t fd) override;
    void OnWrite(std::int32_t fd) override;
    void OnInvalidFd(std::int32_t fd) override;
    void OnConnectionAborted(std::int32_t fd) override;

private:
    int32_t port_;
    bool is_ok_;
    Buffer write_buffer_;
    Buffer read_buffer_;
    std::int32_t socket_fd_;
};

}