#include <server.h>

#include <iostream>

auto main() -> int
{
    startserver(8081);
    char msg_read[1024];
    getmsg(msg_read);
    std::cout << msg_read << std::endl;
}