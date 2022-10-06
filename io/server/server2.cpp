#include "server.h"
#include <fcntl.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <unistd.h>

namespace wss {

void Server::init()
{
    poller_.on_read = [this](int client_fd)
    {
        if (client_fd == server_fd_)
            HandleNewConnection();
        else
            HandleReadAvailable(client_fd);
    };

    poller_.on_write = [this](int client_fd)
    {
        HandleWriteAvailable(client_fd);
    };

    poller_.on_hangup = [this](int client_fd, short flags)
    {
        HandleHangup(client_fd, flags);
    };

    poller_.on_unexpected_event = [this](int client_fd, short flags)
    {
        HandleUnexpectedEvent(client_fd, flags);
    };

    poller_.on_timeout = [this]()
    {
        HandleTimeout();
    };


    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0)
    {
        perror("socket failed");
        Shutdown(EXIT_FAILURE);
    }

    if (fcntl(server_fd_, F_SETFD, O_NONBLOCK) == -1)
    {
        perror("couldn't set non-blocking flag");
        Shutdown(EXIT_FAILURE);
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    int addrlen = sizeof(address);

    if (bind(server_fd_, (sockaddr*)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        Shutdown(EXIT_FAILURE);
    }
    
    if (listen(server_fd_, connection_backlog_) < 0)
    {
        perror("listen");
        Shutdown(EXIT_FAILURE);
    }

    printf("listening for new connections\n");

    poller_.AddFd(server_fd_, Poller::Event::READ);
}

void Server::Run()
{
    assert(server_fd_ != -1);
    poller_.Run();
}

void Server::Shutdown(int exit_status)
{
    for (auto& client : clients_)
    {
        close(client.fd);
        client.shutdown_requested = true;
        if (client.eof_received)
        {
            poller_.RemoveFd(client.fd);
        }
    }
    
    if (exit_status == 1)
    {
        shutdown_timer_enabled_ = true;
        poller_.set_timeout(10);
        poller_.Run();
    }
    else
    {
        if (server_fd_ >= 0 && shutdown(server_fd_, SHUT_RDWR) != 0)
            perror("couldn't shutdown server");
    }
}

void Server::HandleUnexpectedEvent(int client_fd, short flags)
{
    // close connection
}

void Server::HandleTimeout()
{
    if (!shutdown_timer_enabled_)
        assert(false);

    if (server_fd_ >= 0 && shutdown(server_fd_, SHUT_RDWR) != 0)
        perror("couldn't shutdown server");
}

}