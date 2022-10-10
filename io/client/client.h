/*
 * Client - reads user input
 * reads and writes on the socket
 */

#pragma once

#include "../poller.h"
#include "../../buffer.h"

namespace wss {

class Client : public Poller
{
public:

    // Port to connect to
    // Inits buffer
    Client(std::int32_t port);

    // Connect to the server
    void Init();

    bool is_ok() const { return is_ok_; }

    // Call Poller::OnEOF on server_fd
    // Sets is_ok_ to false
    // break Poller::Run loop
    // Register with the singal handler
    void Shutdown();

    // if fd is STDIN calls OnUserInput
    // calls on OnRemoteData
    void OnRead(std::int32_t fd) override;

    // Copies user input to buffer_
    // Update server_fd flag to poll WRITE event
    void OnUserInput(std::int32_t fd);

    // Read data from server fd and print it to STDOUT
    void OnRemoteData(std::int32_t fd);

    // if server_fd and buffer_ not empty
    //      then write the content of buffer into server_fd
    //      if call succeeds and all the bytes are written
    //          then stop polling WRITE event on server_fd
    //          clear the buffer_
    void OnWrite(std::int32_t fd) override;

    // Can STDIO trigger this event?
    // if server_fd, calls Poller::OnHangup
    void OnHangup(std::int32_t fd) override;

    // If server_fd, call Poller::OnEOF
    //      break Poller::Run loop
    void OnEOF(std::int32_t fd) override;

    // if server_fd, call Poller::OnPollnval
    // break Poller::Run loop
    void OnPollnval(std::int32_t fd) override;

private:
    int32_t port_;
    bool is_ok_;
    Buffer buffer_;
};

}