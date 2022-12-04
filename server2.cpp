#include "server2.h"
#include "common.h"
#include <fcntl.h>
#include <unistd.h>

namespace wss {

Server2::Server2(Port port)
    : port_(port)
{
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (is_error(server_fd_, "=> error creating socket"))
    {
        is_ok_ = false;
        return;
    }

    if (is_error(fcntl(server_fd_, F_SETFD, O_NONBLOCK), "=> couldn't set non-blocking flag on server's socket"))
    {
        is_ok_ = false;
        if (is_error(close(server_fd_), "=> error closing server fd"))
            return;
    }

    address_.sin_family = AF_INET;
    address_.sin_addr.s_addr = INADDR_ANY;
    address_.sin_port = htons(port_);

    if (is_error(bind(server_fd_, (sockaddr*)&address_, sizeof(address_)), "=> couldn't bind the socket"))
    {
        is_ok_ = false;
        if (is_error(close(server_fd_), "=> error closing server fd"))
            return;
    }
    
    const std::int32_t backlog = 999;
    if (is_error(listen(server_fd_, backlog), "=> couldn't listen on socket"))
    {
        is_ok_ = false;
        if (is_error(close(server_fd_), "=> error closing server fd"))
            return;
    }

    LOG("listening for new connections\n");

    AddFd(server_fd_, Event::READ);
}

void Server2::Run()
{
    assert(is_ok_);
    auto& fds = fd_list_.fds();

    while(true)
    {
        auto events = poll(fds.data(), fds.size(), -1); // infinite timeouts
        if (is_error(events))
            return;

        // timeout    
        if (events == 0)
        {
            assert(false);
        }

        LOG("%d events received\n", events);

        fd_list_.Lock();
        for (auto it = fds.begin(); it != fds.end() && events > 0; ++it)
        {
            auto& pfd = *it;

            if (pfd.revents == 0)
                continue;

            LogPollEvents(pfd);
                
            if (pfd.revents & POLLIN)
            {
                if (pfd.fd == server_fd_)
                {
                    OnNewConnection(pfd.fd);
                }
                else
                {
                    OnData(pfd.fd);
                }
            }

            if (pfd.revents & POLLHUP)
            {
                assert(false);
            }

            // if (pfd.revents & POLLRDHUP)
            // {
            //     assert(false);
            // }

            if (pfd.revents & POLLNVAL)
            {
                assert(false);
            }

            if (pfd.revents & (POLLERR | POLLPRI))
            {
                assert(false);
            }                

            pfd.revents = 0;
            --events;
        }

        fd_list_.Unlock();
    }
}

void Server2::Shutdown()
{
    LOG("%s\n", "shutting down, closing all sockets");

    for (auto& pfd : fd_list_.fds())
    {
        close(pfd.fd);
    }
    fd_list_.RemoveAll();
}

void Server2::AddFd(Fd fd, Event event)
{
    short flags = 0;
    if (event & Event::READ)
        flags |= POLLIN;
    if (event & Event::WRITE)
        flags |= POLLOUT;
    flags |= POLLRDHUP;

    fd_list_.Add(fd, flags);
}

void Server2::OnNewConnection(Fd fd)
{
    socklen_t addrlen = sizeof(address_);
    auto client_fd = accept4(server_fd_, (sockaddr*)&address_, &addrlen, SOCK_NONBLOCK);
    if (!is_error(client_fd, "=> error accpeting a new connection"))
    {
        LOG("new client connected\n");
        AddFd(client_fd, Event::READ);
    } 
}

void Server2::OnData(Fd fd)
{
    static char buffer[READ_BUFFER_SIZE] = {'\0'};

    auto bytes_read = read(fd, buffer, READ_BUFFER_SIZE - 1);
    is_error(bytes_read, "=> error reading from a client fd");

    assert(bytes_read >= 0);
    if (bytes_read == 0)
    {
        LOG("fd(%d) EOF received\n", fd);
        RemoveFd(fd);
    }
    else
    {
        LOG("bytes_read(%ld)\n", bytes_read);
        buffer[bytes_read] = '\0';
        LOG("%s\n", buffer);
        memset(buffer, '\0', bytes_read);
    }
}

void Server2::RemoveFd(Fd fd)
{
    LOG("closing fd(%d)\n", fd);
    close(fd);
    fd_list_.Remove(fd);
}

}