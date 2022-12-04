#include "server2.h"
#include "common.h"
#include <fcntl.h>
#include <unistd.h>

namespace wss {

Server2::Server2(Port port)
    : port_(port)
{
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (pperror(server_fd_, "=> error creating socket"))
    {
        is_ok_ = false;
        return;
    }

    if (pperror(fcntl(server_fd_, F_SETFD, O_NONBLOCK), "=> couldn't set non-blocking flag on server's socket"))
    {
        is_ok_ = false;
        if (pperror(close(server_fd_), "=> error closing server fd"))
            return;
    }

    address_.sin_family = AF_INET;
    address_.sin_addr.s_addr = INADDR_ANY;
    address_.sin_port = htons(port_);

    if (pperror(bind(server_fd_, (sockaddr*)&address_, sizeof(address_)), "=> couldn't bind the socket"))
    {
        is_ok_ = false;
        if (pperror(close(server_fd_), "=> error closing server fd"))
            return;
    }
    
    const std::int32_t backlog = 999;
    if (pperror(listen(server_fd_, backlog), "=> couldn't listen on socket"))
    {
        is_ok_ = false;
        if (pperror(close(server_fd_), "=> error closing server fd"))
            return;
    }

    LOG("listening for new connections\n");

    AddFd(server_fd_, Event::READ);
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

    LOG("fd(%d) polling for event(%d)\n", fd, event);
}

void Server2::Run()
{
    assert(is_ok_);

    while(true)
    {
        auto& fds = fd_list_.fds();

        auto events = poll(fds.data(), fds.size(), -1); // infinite timeouts
        if (pperror(events))
            return;

        // timeout    
        if (events == 0)
        {
            assert(false);
            return;
        }

        LOG("%d events received\n", events);

        fd_list_.Lock();
        for (auto it = fds.begin(); it != fds.end() && events > 0; ++it)
        {
            auto& pfd = *it;
            LogPollEvents(pfd);

            if (pfd.revents == 0)
                continue;
                
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

            if (pfd.revents & POLLERR || pfd.events & POLLPRI)
            {
                assert(false);
            }

            if (pfd.revents & POLLHUP && !(pfd.revents & POLLIN))
            {
                assert(false);
            }

            if (pfd.events & POLLRDHUP)
            {
                assert(false);
            }

            if (pfd.revents & POLLNVAL)
            {
                assert(false);
            }                

            pfd.revents = 0;
            --events;
        }

        fd_list_.Unlock();
    }
}

void Server2::OnNewConnection(Fd fd)
{
    socklen_t addrlen = sizeof(address_);
    auto client_fd = accept4(server_fd_, (sockaddr*)&address_, &addrlen, SOCK_NONBLOCK);
    if (!pperror(client_fd, "=> error accpeting a new connection"))
    {
        LOG("new client connected\n");
        AddFd(client_fd, Event::READ);
    } 
}

void Server2::OnData(Fd fd)
{
    static char buffer[READ_BUFFER_SIZE];

    auto bytes_read = read(fd, buffer, READ_BUFFER_SIZE - 1);
    pperror(bytes_read, "=> error reading from a client fd");

    assert(bytes_read >= 0);
    if (bytes_read == 0)
    {
        LOG("fd(%d) EOF received", fd);
        return;
    }
    else
    {
        buffer[bytes_read] = '\0';
        LOG("%s", buffer);
    }

    memset(buffer, '\0', bytes_read);
}

}