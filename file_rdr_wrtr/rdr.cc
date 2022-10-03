// read file to open from commandline_args
// user_input:  o -> open the file
//              r -> read from file and print
//              * -> close the file

#include "../utils.h"
#include <cstdio>
#include <fcntl.h>
#include <string>
#include <error.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cassert>

static const constexpr std::size_t BUFFER_SIZE = 1000;
auto fd = -1;
std::vector<char> vec_buffer;
std::string file_path;
char buffer[BUFFER_SIZE];
std::size_t buffer_size = 0;

void OpenFile()
{
    if (fd >= 0)
    {
        printf("file already open!\n");
    }
    else
    {
        fd = open(file_path.c_str(), O_RDONLY);
        if (fd == -1)
        {
            Panic("could not open the file(%s)", file_path.c_str());
        }
    }    
}

void ReadFile()
{
    if (fd < 0)
    {
        fprintf(stderr, "file not open yet\n");
    }
    // assert(buffer.size() == 0);
    // assert(buffer.data() != nullptr);
    auto* write_ptr = vec_buffer.data();
    assert(write_ptr != nullptr);
    auto rv = read(fd, write_ptr, BUFFER_SIZE);
    if (rv == -1)
    {
        fprintf(stderr, "couldn't read date from file\n");
    }
    else
    {
        buffer_size += rv;
        printf("read (%ld) bytes\n", rv);
    }
}

void PrintBuffer()
{
    for (const auto c : vec_buffer)
    {
        std::cout << c;
    }
    // for (int i = 0; i < buffer_size; i++)
    // {
    //     std::cout << *(buffer + i);
    // }
    printf("\n");
    // buffer.clear();
}

void Init(int argc, char** argv)
{
    if (argc != 2)
    {
        Panic("expected one command line argument, (%d) provided", argc);
    }    
    
    file_path = (*(argv + 1));
    vec_buffer.reserve(BUFFER_SIZE);
    vec_buffer.push_back(0);
}

int main(int argc, char** argv)
{
    Init(argc, argv);

    while(true)
    {
        printf(">>> ");
        std::string input;
        std::cin >> input;
        if (input == "o")
        {
            OpenFile();
        }
        else if (input == "r")
        {
            ReadFile();
            PrintBuffer();
        }
        else
        {
            close(fd);
            break;
        }
    }

    return 0;
}