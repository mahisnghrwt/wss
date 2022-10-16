#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "io/common/poller.h"
#include "common/buffer.h"
#include <cstdint>
#include <poll.h>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>

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

   /*
    *   REGISTER WITH SIGNAL HANDLER
    *   Shutdown
    *       for all client_fd:
    *           Poller::OnEOF(client_fd)
    *       Poller::OnEOF(server_fd)
    */
    void ShutdownAll();

private:
    void init();

    /* 
    *   OnRead
    *       if server_fd
    *           OnNewConnection
    *       if client_fd
    *           OnClientData       
    */
    void OnRead(std::int32_t fd) override;

   /* 
    *   OnNewConnection
    *       accept
    *           if fails
    *               log and return early
    *       set O_NONBLOCK flag on client_socket
    *       AddFd for polling
    */
    void OnNewConnection();

   /*
    *   OnClientData
    *       if `read` return value == 0,
    *           desc.eof_received = true
    *           OnEOF
    *       else
    *           check for other errors?     
    */
    void OnClientData(std::int32_t fd);

   /*
    *   OnWrite
    *       return
    */
    void OnWrite(std::int32_t fd) override {}

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
};

}