#include <macrosafe/channel.hpp>

#include <iostream>

auto main() -> int
{
    if (auto message = macrosafe::Channel::receive_message().get(); message)
    {
        std::cout << "Received message: " << message.value() << '\n' << std::flush;
    }
    else
    {
        std::cout << "Failed to receive message\n" << std::flush;
    }
}
