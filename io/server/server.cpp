#include "server.h"
#include "../../utils.h"
#include <fcntl.h>
#include <netinet/ip.h>
#include <unistd.h>

namespace wss {

void Server::init()
{
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (utils::pperror(server_fd_, "=> error creating socket"))
    {
        is_ok_ = false;
        return;
    }

    if (utils::pperror(fcntl(server_fd_, F_SETFD, O_NONBLOCK), "=> couldn't set non-blocking flag on server's socket"))
    {
        is_ok_ = false;
        if (utils::pperror(close(server_fd_), "=> error closing server fd"))
            return;
    }

    address_.sin_family = AF_INET;
    address_.sin_addr.s_addr = INADDR_ANY;
    address_.sin_port = htons(port_);

    if (utils::pperror(bind(server_fd_, (sockaddr*)&address_, sizeof(address_)), "=> couldn't bind the socket"))
    {
        is_ok_ = false;
        if (utils::pperror(close(server_fd_), "=> error closing server fd"))
            return;
    }
    
    if (utils::pperror(listen(server_fd_, connection_backlog_), "=> couldn't listen on socket"))
    {
        is_ok_ = false;
        if (utils::pperror(close(server_fd_), "=> error closing server fd"))
            return;
    }

    printf("=> listening for new connections\n");

    AddFd(server_fd_, Poller::Event::READ);
}

void Server::Run()
{
    if (!is_ok_) return;
    Poller::Run();
}

void Server::Shutdown()
{
    printf("=> shutting down server\n");

    for (std::size_t i = 0; i < fds_.size();)
    {
        const auto size_before = fds_.size();

        if (fds_[i].fd != server_fd_)
            OnEOF(fds_[i].fd);

        if (size_before == fds_.size())
            ++i;
    }

    OnEOF(server_fd_);

    is_ok_ = false;
}

void Server::OnRead(std::int32_t fd)
{
    if (fd == server_fd_)
        OnNewConnection();
    else
        OnClientData(fd);
}

void Server::OnNewConnection()
{
    socklen_t addrlen = sizeof(address_);
    auto client_fd = accept4(server_fd_, (sockaddr*)&address_, &addrlen, SOCK_NONBLOCK);
    if (!utils::pperror(client_fd, "=> error accpeting a new connections"))
    {
        printf("=> new client connected\n");
        AddFd(client_fd, Poller::Event::READ);
    }    
}

void Server::OnClientData(std::int32_t client_fd)
{
    auto bytes_read = read(client_fd, buffer_.head(), buffer_.cap_available());
    utils::pperror(bytes_read, "=> error reading from a client fd");
    if (bytes_read == 0)
    {
        auto* desc = fd_store_->Get(client_fd);
        if (desc != nullptr)
            desc->eof_received = true;
        OnEOF(client_fd);
    }
    else if (bytes_read > 0)
    {
        printf("=> data:\n\t%s", buffer_.data());
    }
    buffer_.Clear();
}

void Server::OnHangup(std::int32_t fd)
{
    if (fd == server_fd_)
        Shutdown();
    else
        Poller::OnHangup(fd);
}

void Server::OnPollnval(std::int32_t fd)
{
    if (fd == server_fd_)
        Shutdown();
    else
        Poller::OnPollnval(fd);
}

}