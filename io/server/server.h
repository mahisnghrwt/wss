#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "io/poller.h"
#include "io/fd_desc.h"
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

    Server(int port, std::size_t connection_backlog, std::size_t max_connections)
        : Poller(max_connections)
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
        Shutdown();
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
    void Shutdown();

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

   /*
    *   OnHangup
    *       if server_fd:
    *           NOT_POSSIBLE
    *       else:
    *           Poller::OnHangup()
    */
    void OnHangup(std::int32_t fd) override;

   /*
    *   OnPollnval:
    *       If server_fd:
    *           NOT_POSSIBLE
    *       else:
    *           Poller::OnPollnval
    */
    void OnPollnval(std::int32_t fd) override;

    const std::int32_t port_;
    const std::size_t connection_backlog_;
    std::int32_t server_fd_;
    Buffer buffer_;
    bool is_ok_;
    sockaddr_in address_;
};

}