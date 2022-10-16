#include "server.h"
#include "common/utils.h"
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

    LOG("listening for new connections\n");

    AddFd(server_fd_, Poller::Event::READ);
}

void Server::Run()
{
    if (!is_ok_) return;
    Poller::Run();
}

void Server::ShutdownAll()
{
    LOG("shutting down server\n");

    for (const auto& client_fd : client_fds_)
    {
        LOG("aborting connection to fd(%d)\n", client_fd);
        Shutdown(client_fd);
    }
    client_fds_.clear();

    Shutdown(server_fd_);
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
    if (!utils::pperror(client_fd, "=> error accpeting a new connection"))
    {
        LOG("new client(%d) connected\n", client_fd);
        client_fds_.emplace_back(client_fd);
        AddFd(client_fd, Poller::Event::READ);
    }    
}

void Server::OnClientData(std::int32_t client_fd)
{
    auto& desc = store_.Add(client_fd);
    if (desc.buffer.cap_available() < 512)
        desc.buffer.reserve(desc.buffer.size() + 1024);
    
    auto bytes_read = read(client_fd, desc.buffer.head(), desc.buffer.cap_available());
    utils::pperror(bytes_read, "=> error reading from a client fd");
    if (bytes_read == 0)
    {
        LOG("fd(%d) EOF received\n", client_fd);
        Shutdown(client_fd);
    }
    else if (bytes_read > 0)
    {
        LOG("data:[%s]\n", desc.buffer.head());
        desc.buffer.move_head(bytes_read);
        UpdateFd(client_fd, READ | WRITE);
    }
}

void Server::OnWrite(std::int32_t fd)
{
    if (fd == server_fd_)
    {
        ShutdownAll();
    }
    else
    {
        auto* desc = store_.Find(fd);
        if (desc != nullptr && desc->buffer.size() > 0)
        {
            auto bytes_written = send(fd, desc->buffer.data(), desc->buffer.size(), 0);

            LOG("fd(%d) (%ld) bytes written\n", fd, bytes_written);
            
            if (utils::pperror(bytes_written, "=> error writing to fd"))
                return;

            if (bytes_written == desc->buffer.size())
            {
                desc->buffer.Clear();
                UpdateFd(fd, READ);
            }
            else if (bytes_written > 0)
            {
                assert(bytes_written < desc->buffer.size());

                char* new_data = new char[desc->buffer.size()];
                
                auto remaining_data_size = desc->buffer.size() - bytes_written;
                LOG("fd(%d) (%ld) bytes pending to be written\n", fd, remaining_data_size);
                memcpy(new_data, desc->buffer.data() + bytes_written, remaining_data_size);
                
                desc->buffer.Clear();
                memcpy(desc->buffer.head(), new_data, remaining_data_size);
                desc->buffer.move_head(remaining_data_size);

                delete[] new_data;
            }
        }
        else
        {
            UpdateFd(fd, READ);
        }
    }
}

void Server::OnInvalidFd(std::int32_t fd)
{
    if (fd == server_fd_)
    {
        ShutdownAll();
    }
    else
    {
        utils::remove_if(client_fds_, [fd](auto& f) { return f == fd; });
        RemoveFd(fd);
    }
}

void Server::OnConnectionAborted(std::int32_t fd)
{
    OnInvalidFd(fd);
}

void Server::Shutdown(std::int32_t fd)
{
    shutdown(fd, SHUT_RDWR);
    close(fd);
    RemoveFd(fd);
}

}