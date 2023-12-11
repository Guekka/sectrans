#include <macrosafe/common.hpp>
#include <macrosafe/server.hpp>

#include <iostream>

auto main() -> int
{
    const macrosafe::Server server;
    std::cout << "Server started on port " << macrosafe::k_port << '\n' << std::flush;

    if (const auto message = server.receive_message_blocking(); message.has_value())
    {
        std::cout << "Received message: " << *message << '\n' << std::flush;
    }
    else
    {
        std::cout << "Failed to receive message\n" << std::flush;
    }
}
