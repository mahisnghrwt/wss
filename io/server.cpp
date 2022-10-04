#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include "../buffer.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int server_fd, socket_fd = -1;

void signal_handler(int signal)
{
    if (socket_fd >= 0 && close(socket_fd) != 0)
        perror("couldn't close socket");

    if (server_fd >= 0 && shutdown(server_fd, SHUT_RDWR) != 0)
        perror("couldn't shutdown server");

    printf("singal caught(%d)", signal);
}

int main(int argc, char const* argv[])
{
    signal(SIGINT, signal_handler);

    Buffer buffer(BUFFER_SIZE);

	// Creating socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
    {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

    sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
    int addrlen = sizeof(address);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0)
    {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	
    if (listen(server_fd, 3) < 0)
    {
		perror("listen");
		exit(EXIT_FAILURE);
	}

    printf("listening for new connections\n");
	
    socket_fd = accept(server_fd, (sockaddr*)&address, (socklen_t*)&addrlen);
    if (socket_fd < 0)
    {
		perror("accept");
		exit(EXIT_FAILURE);
	}

    printf("client connected\n");

    while(true)
    {
        int valread = read(socket_fd, buffer.head(), BUFFER_SIZE);
	    printf("%s\n", buffer.data());
        buffer.Clear();
    }	

	close(socket_fd);

	shutdown(server_fd, SHUT_RDWR);
	return 0;
}
