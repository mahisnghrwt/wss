/*
 * Client - reads user input
 * reads and writes on the socket
 */

#pragma once

#include "io/poller.h"
#include "common/buffer.h"

namespace wss {

/*
 *
 *          server              client
 * EOF      shutdown_all        - init shutdown on server & wait for EOF
 *                              - remove STDIN fd
 * SHUTDOWN shutdown_all        - remove stdin fd
 *                              - flush the buffer
 *                              - init shutdown on server & wait for EOF
 */

class Client : public Poller
{
public:
    const std::size_t BUFFER_SIZE = 1024;

    // Port to connect to
    // Inits buffer
    // start listening on server and stdin fd
    Client(std::int32_t port);

    // Connect to the server
    void Init();

    void Run() { Poller::Run(); }

    bool is_ok() const { return is_ok_; }

    // if fd is STDIN
    //      Copies user input to buffer_
    //      Update server_fd flag to poll WRITE event     
    // else
    //      Read data from server fd and print it to STDOUT
    void OnRead(std::int32_t fd) override;

    // if server_fd and buffer_ not empty
    //      then write the content of buffer into server_fd
    //      if call succeeds and all the bytes are written
    //          then stop polling WRITE event on server_fd
    //          clear the buffer_
    void OnWrite(std::int32_t fd) override;

    // Call Poller::OnEOF on server_fd
    // Sets is_ok_ to false
    // break Poller::Run loop
    // Register with the singal handler
    void Shutdown();

    // Can STDIO trigger this event?
    // if server_fd, calls Poller::OnHangup
    void OnHangup(std::int32_t fd) override;

    // // If server_fd, call Poller::OnEOF
    // //      break Poller::Run loop
    // void OnEOF(std::int32_t fd) override;

    // if server_fd, call Poller::OnPollnval
    // break Poller::Run loop
    void OnPollnval(std::int32_t fd) override;

private:
    int32_t port_;
    bool is_ok_;
    Buffer write_buffer_;
    Buffer read_buffer_;
    std::int32_t socket_fd_;
};

}