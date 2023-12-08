#include "doctest.h"

#include <macrosafe/client.hpp>
#include <macrosafe/server.hpp>

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
}