#include "utils.h"

void test_1()
{
    LOG("Hello World!\n");
}

void test_2()
{
    LOG_DEBUG("Debug msg\n")
}

int main()
{
    test_1();
    test_2();
    return 0;
}