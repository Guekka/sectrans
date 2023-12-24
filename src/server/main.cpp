#include <macrosafe/channel.hpp>
#include <macrosafe/detail/common.hpp>

#include <cassert>
#include <iostream>

auto main() -> int
{
    auto channel = macrosafe::Channel{macrosafe::k_default_server_port, macrosafe::k_default_client_port};
    if (auto message = channel.receive_message_blocking(); message)
    {
        std::cout << "Received message: " << message.value() << '\n' << std::flush;
    }
    else
    {
        std::cout << "Failed to receive message\n" << std::flush;
    }
}
