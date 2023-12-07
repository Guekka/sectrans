#include <client.h>

auto main() -> int
{
    sndmsg(const_cast<char *>("Hello, World!"), 8081);
}