#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <poll.h>
#include <unistd.h>

int main()
{
    auto* fds = new pollfd[1];
    std::size_t nfds = 1;

    const constexpr std::size_t buffer_size = 64;
    char buffer[buffer_size];
    memset(buffer, '\0', buffer_size);

    fds->events = POLLIN;
    fds->fd = STDIN_FILENO;

    while(true)
    {
        int rv = poll(fds, nfds, -1);
        if (rv == -1)
        {
            perror("");
            return 0;
        }

        assert(rv == 1);

        if (fds->revents & POLLIN)
        {
            auto bytes_read = read(STDIN_FILENO, buffer, buffer_size);
            if (bytes_read == -1)
            {
                perror("");
                return 0;
            }

            printf("data: [%s]\n", buffer);
            memset(buffer, '\0', buffer_size);
        }        
        fds->revents = 0;
    }

    delete[] fds;

    return 0;
}