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

    // clean client_fds and server_fd
    // ctor:
    //      port
    //      
    // run: start listening
    //      poll server_fd and client_fds for read
    //      POLLIN
    //      POLLRDHUP (define _GNU_SOURCE before including any header file) // ignnore for now
    //      POLLERR
    //      POLLHUP, peer closed connection, but continue reading until read() call returns 0
    //          meaninig END OF FILE


private:
    void HandleReadAvailable(int client_fd);
    void HandleWriteAvailable(int client_fd);
    void HandleNewConnection();
    void HandleHangup(int client_fd, short flags);
    void HandleUnexpectedEvent(int client_fd, short flags);
    void HandleTimeout();

    // 
    // OnNewConnection
    //      set NONBLOCK flag
    //      { socket_fd, id}
    // OnConnectionClose
    //      clean client's state
    //      send heartbeats to client, if return value for write is -1, means connection is closed
    // OnClientMsg
    //      print data
    //      clean the buffer
    //      if read's return value is 0, means client disconnected
    // shutdown
    //      close all connection
    //      but continue to listen on them until we receive EOF
    //      register this with signal handler
private:
    const int port_;
    const std::size_t connection_backlog_;
    int server_fd_;
    Buffer buffer_;
    std::vector<ClientDesc> clients_;
    std::size_t client_counter_;
    std::size_t max_connections_;
    Poller poller_;
    bool shutdown_timer_enabled_;
};

}