#include "client.h"
#include "common/utils.h"
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

namespace wss {

Client::Client(std::int32_t port)
    : Poller()
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
    socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (utils::pperror(socket_fd_))
        return;

	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_);

    const std::string ip("127.0.0.1");

    if (utils::pperror(inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr)))
        return;

    if (utils::pperror(connect(socket_fd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "=> couldn't connect to the server"))
    {
        return;
    }
    else
    {
        LOG("connected to the server(%s:%d)\n", ip.c_str(), port_);
        is_ok_ = true;
    }

    if (utils::pperror(fcntl(socket_fd_, F_SETFL, fcntl(socket_fd_, F_GETFL, 0) | O_NONBLOCK), "=> couldn't set the nonblocking flag"))
        return;

    // O_NONBLOCK flag not required for STDIN?

    AddFd(STDIN_FILENO, READ);
    AddFd(socket_fd_, READ);    
}

void Client::OnRead(std::int32_t fd)
{
    Buffer* buffer = nullptr;
    
    if (fd == STDIN_FILENO)
        buffer = &write_buffer_;
    else
        buffer = &read_buffer_;

    if (buffer->cap_available() < 512)
        buffer->reserve(buffer->size() + 1024);

    auto bytes_read = read(fd, buffer->head(), buffer->cap_available());
    LOG("%d bytes read\n", bytes_read);


    if (fd == STDIN_FILENO)
    {
        if (utils::pperror(bytes_read, "=> error reading bytes"))
        {
            Shutdown();
        }
        else if (bytes_read > 0)
        {
            // last bytes in the new-line character
            write_buffer_.move_head(bytes_read - 1);           
            *(write_buffer_.head()) = '\0';

            UpdateFd(socket_fd_, READ | WRITE);
        }
    }
    else
    {
        if (utils::pperror(bytes_read) || bytes_read == 0)
        {
            Shutdown();
        }
        else
        {
            LOG("%d bytes read\n", bytes_read);
            LOG("data: [%s]\n", read_buffer_.data());
            read_buffer_.Clear();
        }        
    }
}

void Client::OnWrite(std::int32_t fd)
{
    if (fd == STDIN_FILENO)
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

            if (utils::pperror(bytes_written, "=> error writing to the socket_fd"))
                return;

            assert(bytes_written <= write_buffer_.size());

            if (bytes_written < write_buffer_.size())
            {
                char* new_data = new char[write_buffer_.size()];
                
                auto remaining_data_size = write_buffer_.size() - bytes_written;
                LOG("fd(%d) (%ld) bytes pending to be written\n", fd, remaining_data_size);
                memcpy(new_data, write_buffer_.data() + bytes_written, remaining_data_size);
                
                write_buffer_.Clear();
                memcpy(write_buffer_.head(), new_data, remaining_data_size);
                write_buffer_.move_head(remaining_data_size);

                delete[] new_data;
            }
            else
            {
                // all the data is sent
                UpdateFd(fd, READ);
                write_buffer_.Clear();
            }
        }
        else
        {
            UpdateFd(fd, READ);
            write_buffer_.Clear();
        }
    }
}

void Client::OnInvalidFd(std::int32_t fd)
{
    Shutdown();
}

void Client::OnConnectionAborted(std::int32_t fd)
{
    Shutdown();
}

void Client::Shutdown()
{
    LOG("shutting down\n");

    shutdown(socket_fd_, SHUT_RDWR);
    close(socket_fd_);

    RemoveFd(socket_fd_);
    RemoveFd(STDIN_FILENO);

    is_ok_ = false;
}

}