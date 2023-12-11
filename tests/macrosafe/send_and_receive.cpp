#include "doctest.h"

#include <macrosafe/client.hpp>
#include <macrosafe/server.hpp>
#include <macrosafe/utils/threading.hpp>

#include <thread>

TEST_CASE("send and receive")
{
    SUBCASE("basic")
    {
        auto server = macrosafe::Server{};

        REQUIRE_EQ(macrosafe::send_message("hello"), macrosafe::SendResult::Success);
        const auto message = server.receive_message_blocking();

        REQUIRE_EQ(message, std::optional<std::string>{"hello"});
    }

    SUBCASE("multiple messages")
    {
        auto server = macrosafe::Server{};

        REQUIRE_EQ(macrosafe::send_message("hello"), macrosafe::SendResult::Success);
        REQUIRE_EQ(macrosafe::send_message("world"), macrosafe::SendResult::Success);
        const auto message1 = server.receive_message_blocking();
        const auto message2 = server.receive_message_blocking();

        REQUIRE_EQ(message1, std::optional<std::string>{"hello"});
        REQUIRE_EQ(message2, std::optional<std::string>{"world"});
    }

    SUBCASE("empty message")
    {
        auto server = macrosafe::Server{};

        REQUIRE_EQ(macrosafe::send_message(""), macrosafe::SendResult::Success);
        const auto message = server.receive_message_blocking();

        REQUIRE_EQ(message, std::optional<std::string>{""});
    }

    SUBCASE("message containing null character")
    {
        auto server = macrosafe::Server{};

        REQUIRE_EQ(macrosafe::send_message("hello\0world"), macrosafe::SendResult::Success);
        const auto message = server.receive_message_blocking();

        REQUIRE_EQ(message, std::optional<std::string>{"hello\0world"});
    }

    SUBCASE("long message")
    {
        auto server = macrosafe::Server{};

        const auto message = std::string(99999, 'a');

        // we need to start receiving before sending, otherwise we will overflow the queue
        auto thread = macrosafe::JThread([&server, &message]() {
            const auto received_message = server.receive_message_blocking();
            REQUIRE_EQ(received_message.value().size(), message.size());
            REQUIRE_EQ(received_message.value(), message);
        });

        REQUIRE_EQ(macrosafe::send_message(message), macrosafe::SendResult::Success);
    }
}
