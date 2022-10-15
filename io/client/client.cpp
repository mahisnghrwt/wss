#include "client.h"
#include "../../utils.h"
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

namespace wss {

Client::Client(std::int32_t port)
    : Poller(10)
    , port_(port)
    , is_ok_(false)
    , write_buffer_(BUFFER_SIZE)
    , read_buffer_(BUFFER_SIZE)
    , socket_fd_(-1)
{
    Init();
}

void Client::Init()
{
    // TODO: set nonblock flag on both stdin and socket_fd_
    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (utils::pperror(socket_fd_))
        return;

	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_);

    if (utils::pperror(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)))
        return;

    if (!utils::pperror(connect(socket_fd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "couldn't connect to the server"))
    {
        LOG("connected to the server\n");
        is_ok_ = true;
    }
    else
    {
        return;
    }

    if (utils::pperror(fcntl(socket_fd_, F_SETFL, fcntl(socket_fd_, F_GETFL, 0) | O_NONBLOCK), "couldn't set the nonblocking flag"))
        return;

    if (!AddFd(STDIN_FILENO, READ))
    {
        LOG("couldn't add fd\n");
    }
        
    if (!AddFd(socket_fd_, READ))
    {
        LOG("couldn't add fd\n");
    }        
}

void Client::OnRead(std::int32_t fd)
{
    LOG("fd(%d) called\n", fd);
    if (fd == 0)
    {
        auto bytes_read = read(0, write_buffer_.head(), write_buffer_.cap_available());
        LOG("%d bytes read\n", bytes_read);
        if (utils::pperror(bytes_read))
        {
            Shutdown();
            return;
        }

        if (bytes_read > 0)
        {
            // last bytes in the new-line character
            write_buffer_.move_head(bytes_read - 1);           
            *(write_buffer_.head()) = '\0';

            bool socket_fd_found = false;
            for (auto& fd_desc : fds_)
            {
                if (fd_desc.fd == socket_fd_)
                {
                    fd_desc.events |= POLLOUT;
                    socket_fd_found = true;
                    break;
                }
            }
            if (!socket_fd_found)
            {
                LOG("socket_fd(%d) not found in fds_\n", socket_fd_);
                Shutdown();
                assert(false);
            }
        }
    }
    else
    {
        auto bytes_read = read(0, read_buffer_.head(), read_buffer_.cap_available());
        if (utils::pperror(bytes_read) || bytes_read == 0)
        {
            Shutdown();
        }
        else
        {
            LOG("%d bytes read\n", bytes_read);
            LOG("data: [%s]", read_buffer_.data());
            read_buffer_.Clear();
        }        
    }
}

void Client::OnWrite(std::int32_t fd)
{
    if (fd == 0)
    {
        LOG("received write event on STDIN\n");
        Shutdown();
    }
    else
    {
        assert(fd == socket_fd_);
        LOG_DEBUG("data in write_buffer: [%s]\n", write_buffer_.data());
        if (write_buffer_.size() > 0)
        {
            auto bytes_written = write(fd, write_buffer_.data(), write_buffer_.size());
            LOG("fd(%d) (%d) bytes written\n", fd, bytes_written);
            if (utils::pperror(bytes_written, "error writing to the socket_fd"))
            {
                Shutdown();
                return;
            }
        }

        write_buffer_.Clear();

        bool socket_fd_found = false;
        for (auto& fd_desc : fds_)
        {
            if (fd_desc.fd == socket_fd_)
            {
                std::int32_t mask = ~POLLOUT;
                fd_desc.events &= mask;
                socket_fd_found = true;
                break;
            }
        }

        if (!socket_fd_found)
        {
            LOG("socket_fd(%d) not found in fds_\n", socket_fd_);
            Shutdown();
            assert(false);
        }
    }
}

void Client::Shutdown()
{
    OnEOF(0);
    OnEOF(socket_fd_);
    is_ok_ = false;
}

void Client::OnHangup(std::int32_t fd)
{
    if (fd != STDIN_FILENO)
        Shutdown();
}

void Client::OnPollnval(std::int32_t fd)
{
    Shutdown();
}

}