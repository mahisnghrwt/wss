#pragma once

#include "../../buffer.h"
#include "../poller.h"
#include <cstdint>
#include <poll.h>
#include <vector>

namespace wss {

struct ClientDesc
{
    std::uint64_t id;
    int fd;
    bool shutdown_requested;
    bool eof_received;
};

class Server
{
public:
    static const constexpr std::size_t BUFFER_SIZE = 1024;

    Server(int port, std::size_t connection_backlog, std::size_t max_connections)
        : port_(port)
        , connection_backlog_(connection_backlog)
        , server_fd_(-1)
        , buffer_(BUFFER_SIZE)
        , client_counter_(0)
        , max_connections_(max_connections)
        , poller_(max_connections)
        , shutdown_timer_enabled_(false)
    {
        init();
    }

    void init();
    void Run();
    void Shutdown(int exit_status);

    /* 
    *   OnRead
    *       if server_fd
    *           OnNewConnection
    *       if client_fd
    *           OnClientData       
    */

   /* 
    *   OnNewConnection
    *       accept
    *           if fails
    *               log and return early
    *       set O_NONBLOCK flag on client_socket
    *       AddFd for polling
    */

   /*
    *   OnClientData
    *       if `read` return value == 0,
    *           desc.eof_received = true
    *           OnEOF
    *       else
    *           check for other errors?     
    */

   /*
    *   OnWrite
    *       return
    */

   /*
    *   OnHangup
    *       if server_fd:
    *           NOT_POSSIBLE
    *       else:
    *           Poller::OnHangup()
    */

   /*
    *   OnEOF:
    *       if server_fd:
    *           NOT_POSSIBLE
    *       else:
    *           Poller::EOF
    */

   /*
    *   OnPollnval:
    *       If server_fd:
    *           NOT_POSSIBLE
    *       else:
    *           Poller::OnPollnval
    */

   /*
    *   REGISTER WITH SIGNAL HANDLER
    *   Shutdown
    *       for all client_fd:
    *           Poller::OnEOF(client_fd)
    *       Poller::OnEOF(server_fd)
    */

private:
    void HandleReadAvailable(int client_fd);
    void HandleWriteAvailable(int client_fd);
    void HandleNewConnection();
    void HandleHangup(int client_fd, short flags);
    void HandleUnexpectedEvent(int client_fd, short flags);
    void HandleTimeout();
    
private:
    const std::int32_t port_;
    const std::size_t connection_backlog_;
    std::int32_t server_fd_;
    Buffer buffer_;
    std::vector<ClientDesc> clients_;
    std::size_t client_counter_;
    std::size_t max_connections_;
    Poller poller_;
    bool shutdown_timer_enabled_;
};

}