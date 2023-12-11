#include "doctest.h"

#include <macrosafe/channel.hpp>
#include <macrosafe/utils/threading.hpp>

#include <thread>

using macrosafe::Channel;

TEST_CASE("send and receive")
{
    SUBCASE("basic")
    {
        REQUIRE_EQ(Channel::send_message("hello"), macrosafe::SendResult::Success);
        const auto message = Channel::receive_message_blocking();

        REQUIRE_EQ(message, std::optional<std::string>{"hello"});
    }

    SUBCASE("multiple messages")
    {
        REQUIRE_EQ(Channel::send_message("hello"), macrosafe::SendResult::Success);
        REQUIRE_EQ(Channel::send_message("world"), macrosafe::SendResult::Success);
        const auto message1 = Channel::receive_message_blocking();
        const auto message2 = Channel::receive_message_blocking();

        REQUIRE_EQ(message1, std::optional<std::string>{"hello"});
        REQUIRE_EQ(message2, std::optional<std::string>{"world"});
    }

    SUBCASE("empty message")
    {
        REQUIRE_EQ(Channel::send_message(""), macrosafe::SendResult::Success);
        const auto message = Channel::receive_message_blocking();

        REQUIRE_EQ(message, std::optional<std::string>{""});
    }

    SUBCASE("message containing null character")
    {
        REQUIRE_EQ(Channel::send_message("hello\0world"), macrosafe::SendResult::Success);
        const auto message = Channel::receive_message_blocking();

        REQUIRE_EQ(message, std::optional<std::string>{"hello\0world"});
    }

    SUBCASE("long message")
    {
        const auto message = std::string(99999, 'a');

        // we need to start receiving before sending, otherwise we will overflow the queue
        auto received_message_future = Channel::receive_message();
        REQUIRE_EQ(Channel::send_message(message), macrosafe::SendResult::Success);

        const auto received_message = received_message_future.get();

        REQUIRE_EQ(received_message.value().size(), message.size());
        REQUIRE_EQ(received_message.value(), message);
    }
}
