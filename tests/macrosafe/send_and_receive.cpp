#include "doctest.h"

#include <macrosafe/channel.hpp>
#include <macrosafe/utils/threading.hpp>

#include <thread>

using macrosafe::Channel;

constexpr auto k_server_port = 12345;
constexpr auto k_client_port = 12346;

TEST_CASE("send and receive")
{
    auto server_channel = Channel{k_server_port, k_client_port};
    auto client_channel = Channel{k_client_port, k_server_port};

    SUBCASE("basic")
    {
        REQUIRE_EQ(client_channel.send_message("hello"), macrosafe::SendResult::Success);
        const auto message = server_channel.receive_message_blocking();

        REQUIRE_EQ(message, std::optional<std::string>{"hello"});
    }

    SUBCASE("multiple messages")
    {
        REQUIRE_EQ(client_channel.send_message("hello"), macrosafe::SendResult::Success);
        REQUIRE_EQ(client_channel.send_message("world"), macrosafe::SendResult::Success);
        const auto message1 = server_channel.receive_message_blocking();
        const auto message2 = server_channel.receive_message_blocking();

        REQUIRE_EQ(message1, std::optional<std::string>{"hello"});
        REQUIRE_EQ(message2, std::optional<std::string>{"world"});
    }

    SUBCASE("empty message")
    {
        REQUIRE_EQ(client_channel.send_message(""), macrosafe::SendResult::Success);
        const auto message = server_channel.receive_message_blocking();

        REQUIRE_EQ(message, std::optional<std::string>{""});
    }

    SUBCASE("message containing null character")
    {
        REQUIRE_EQ(client_channel.send_message("hello\0world"), macrosafe::SendResult::Success);
        const auto message = server_channel.receive_message_blocking();

        REQUIRE_EQ(message, std::optional<std::string>{"hello\0world"});
    }

    SUBCASE("long message")
    {
        const auto message = std::string(99999, 'a');

        // we need to start receiving before sending, otherwise we will overflow the queue
        auto received_message_future = server_channel.receive_message();
        REQUIRE_EQ(client_channel.send_message(message), macrosafe::SendResult::Success);

        const auto received_message = received_message_future.get();

        REQUIRE_EQ(received_message.value().size(), message.size());
        REQUIRE_EQ(received_message.value(), message);
    }
}
