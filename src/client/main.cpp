#include <macrosafe/channel.hpp>
#include <macrosafe/detail/common.hpp>

#include <cassert>

auto main() -> int
{
    auto channel = macrosafe::Channel{macrosafe::k_default_client_port, macrosafe::k_default_server_port};
    assert(channel.send_message("Hello, world!") == macrosafe::SendResult::Success);
    std::cout << channel.receive_message_blocking().value() << '\n';
}
