#include "client2.h"
#include "common.h"
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

namespace wss {

Client2::Client2(Port port)
    : port_(port)
{
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (pperror(fd_))
        return;

	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_);

    const std::string ip("127.0.0.1");

    if (pperror(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)))
        return;

    if (pperror(connect(fd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "couldn't connect to the server"))
    {
        return;
    }
    else
    {
        LOG("connected to the server(%s:%d)\n", ip.c_str(), port_);
        is_ok_ = true;
    }

    if (pperror(fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL, 0) | O_NONBLOCK), "couldn't set the nonblocking flag"))
        return;

    // O_NONBLOCK flag not required for STDIN?

    AddFd(STDIN_FILENO, READ);
    AddFd(fd_, READ);
}

void Client2::AddFd(Fd fd, Event event)
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

void Client2::Run()
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

            if (pfd.revents == 0)
                continue;

            LogPollEvents(pfd);
                
            if (pfd.revents & POLLIN)
            {
                OnData(pfd.fd);
            }

            if (pfd.revents & POLLOUT)
            {
                OnWriteData(pfd.fd);
            }

            if (pfd.revents & POLLERR || pfd.events & POLLPRI)
            {
                assert(false);
            }

            if (pfd.revents & POLLHUP && !(pfd.revents & POLLIN))
            {
                assert(false);
            }

            if (pfd.revents & POLLRDHUP && !(pfd.revents & POLLIN))
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

void Client2::OnData(Fd fd)
{
    LOG("fd(%d) called\n", fd);
    if (fd == 0)
    {
        auto len = strlen(write_buffer_);
        assert(len <= WRITE_BUFFER_SIZE);
        auto available_cap = WRITE_BUFFER_SIZE - len;
        if (available_cap == 0)
        {
            assert(false);
        }

        if (len > 0)
        {
            write_buffer_[len - 1] = '\n';
        }

        auto bytes_read = read(STDIN_FILENO, write_buffer_ + len, available_cap);
        LOG("%d bytes read\n", bytes_read);
        if (pperror(bytes_read))
        {
            Shutdown();
            return;
        }

        if (bytes_read > 0)
        {
            // last character read is a newline character
            write_buffer_[bytes_read - 1] = '\0';
            fd_list_.Update(fd_, POLLIN | POLLOUT | POLLRDHUP);
        }
    }
    else
    {
        auto bytes_read = read(0, read_buffer_, READ_BUFFER_SIZE - 1);
        if (pperror(bytes_read) || bytes_read == 0)
        {
            Shutdown();
        }
        else
        {
            assert(bytes_read > 0);
            read_buffer_[bytes_read] = '\0';
            LOG("bytes_read: [%d] data: [%s]", bytes_read, read_buffer_);
            memset(read_buffer_, '\0', bytes_read);
        }        
    }
}

void Client2::OnWriteData(Fd fd)
{
    assert(fd == fd_);
    LOG("data in write_buffer: [%s]\n", write_buffer_);
    auto len = strlen(write_buffer_);

    if (len > 0)
    {
        auto bytes_written = write(fd, write_buffer_, len);
        LOG("fd(%d) (%d) bytes written\n", fd, bytes_written);
        if (pperror(bytes_written, "error writing to the socket_fd"))
        {
            Shutdown();
            return;
        }
    }

    memset(write_buffer_, '\0', WRITE_BUFFER_SIZE);
    fd_list_.Update(fd_, POLLIN | POLLRDHUP);
}

}