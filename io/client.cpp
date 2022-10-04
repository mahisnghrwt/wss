#include "../buffer.h"
#include <cassert>
#include <iostream>
#include <signal.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int socket_fd = -1;
int client_fd = -1;

void signal_handler(int signal)
{
    if (client_fd >= 0 && close(client_fd) != 0)
        perror("");
    printf("singal caught(%d)", signal);
}

void connect_to_server()
{    
    const int server_port = 8080;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0)
    {
        perror("");
        return;
    }

	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(server_port);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
		perror("");
        return;
	}

    client_fd = connect(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	if (client_fd < 0)
	{
		perror("");
        return;
	}
}

void tty()
{
    assert(socket_fd >= 0);
    while(true)
    {
        std::string buffer;
        std::cout << ">>> ";
        std::cin >> buffer;
        auto bytes_written = send(socket_fd, buffer.c_str(), buffer.size(), 0);
        if (bytes_written < 0)
            perror("error sending data");
        else
            printf("(%d) bytes written\n", bytes_written);
    }
}


int main(int argc, char const* argv[])
{
    signal(SIGINT, signal_handler);

	connect_to_server();

    if (socket_fd < 0 || client_fd < 0)
        return 1;

    tty();

	// send(sock, hello, strlen(hello), 0);
	// printf("Hello message sent\n");
	// valread = read(sock, buffer, 1024);
	// printf("%s\n", buffer);

	// closing the connected socket
	close(client_fd);
	return 0;
}
