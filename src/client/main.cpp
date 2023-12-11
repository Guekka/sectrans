#include <macrosafe/channel.hpp>

#include <cassert>

auto main() -> int
{
    assert(macrosafe::Channel::send_message("Hello, world!") == macrosafe::SendResult::Success);
}
