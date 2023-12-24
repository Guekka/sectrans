#include "doctest.h"

#include <macrosafe/channel.hpp>
#include <macrosafe/utils/threading.hpp>

#include <thread>

using macrosafe::Channel;

constexpr auto k_server_port = 12345;
constexpr auto k_client_port = 12346;

[[nodiscard]] auto as_bytes(std::string_view str) -> std::vector<std::byte>
{
    return {std::launder(reinterpret_cast<const std::byte *>(str.data())),
            std::launder(reinterpret_cast<const std::byte *>(str.data() + str.size()))};
}

TEST_CASE("send and receive")
{
    auto server_channel = Channel{k_server_port, k_client_port};
    auto client_channel = Channel{k_client_port, k_server_port};

    SUBCASE("basic")
    {
        REQUIRE_EQ(client_channel.send_message(as_bytes("hello")), macrosafe::SendResult::Success);
        const auto message = server_channel.receive_message_blocking();

        REQUIRE_EQ(message, std::optional<std::vector<std::byte>>{as_bytes("hello")});
    }

    SUBCASE("multiple messages")
    {
        REQUIRE_EQ(client_channel.send_message(as_bytes("hello")), macrosafe::SendResult::Success);
        REQUIRE_EQ(client_channel.send_message(as_bytes("world")), macrosafe::SendResult::Success);
        const auto message1 = server_channel.receive_message_blocking();
        const auto message2 = server_channel.receive_message_blocking();

        REQUIRE_EQ(message1, std::optional<std::vector<std::byte>>{as_bytes("hello")});
        REQUIRE_EQ(message2, std::optional<std::vector<std::byte>>{as_bytes("world")});
    }

    SUBCASE("empty message")
    {
        REQUIRE_EQ(client_channel.send_message({}), macrosafe::SendResult::Success);
        const auto message = server_channel.receive_message_blocking();

        REQUIRE_EQ(message, std::optional<std::vector<std::byte>>{});
    }

    SUBCASE("message containing null character")
    {
        REQUIRE_EQ(client_channel.send_message(as_bytes("hello\0world")), macrosafe::SendResult::Success);
        const auto message = server_channel.receive_message_blocking();

        REQUIRE_EQ(message, std::optional<std::vector<std::byte>>{as_bytes("hello\0world")});
    }

    SUBCASE("long message")
    {
        const auto message = std::vector<std::byte>(1024LL * 1024LL, std::byte{0x42});

        // we need to start receiving before sending, otherwise we will overflow the queue
        auto received_message_future = server_channel.receive_message();
        REQUIRE_EQ(client_channel.send_message(message), macrosafe::SendResult::Success);

        const auto received_message = received_message_future.get();

        REQUIRE_EQ(received_message.value().size(), message.size());
        REQUIRE_EQ(received_message.value(), message);
    }
}
