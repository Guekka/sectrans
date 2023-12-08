#include <macrosafe/client.hpp>

#include <cassert>

auto main() -> int
{
    assert(macrosafe::send_message("Hello, world!") == macrosafe::SendResult::Success);
}