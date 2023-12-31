#include "doctest.h"

#include <macrosafe/channel.hpp>
#include <macrosafe/encrypted_channel.hpp>
#include <macrosafe/utils/threading.hpp>

#include <list>
#include <sstream>

using macrosafe::Channel;

constexpr auto k_server_port = 12345;
constexpr auto k_client_port = 12346;

[[nodiscard]] auto as_bytes(std::string_view str) -> std::vector<std::byte>
{
    return {std::launder(reinterpret_cast<const std::byte *>(str.data())),
            std::launder(reinterpret_cast<const std::byte *>(str.data() + str.size()))};
}

namespace doctest {
template<typename T>
struct StringMaker<std::optional<T>>
{
    static String convert(const std::optional<T> &in)
    {
        if (in.has_value())
        {
            return StringMaker<T>::convert(in.value());
        }
        else
        {
            return "nullopt";
        }
    }
};

template<>
struct StringMaker<std::byte>
{
    static String convert(const std::byte &in)
    {
        std::ostringstream oss;
        oss << "std::byte{" << static_cast<int>(in) << "}";
        return oss.str().c_str();
    }
};

template<typename T>
struct StringMaker<std::vector<T>>
{
    static String convert(const std::vector<T> &in)
    {
        std::ostringstream oss;
        oss << "std::vector{" << typeid(T).name() << "}{";
        for (auto elem : in)
        {
            oss << StringMaker<T>::convert(elem) << ", ";
        }
        oss << "}";
        return oss.str().c_str();
    }
};

} // namespace doctest

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

        REQUIRE_EQ(message, std::vector<std::byte>{});
    }

    SUBCASE("message containing null character")
    {
        REQUIRE_EQ(client_channel.send_message(as_bytes("hello\0world")), macrosafe::SendResult::Success);
        const auto message = server_channel.receive_message_blocking();

        REQUIRE_EQ(message, std::optional<std::vector<std::byte>>{as_bytes("hello\0world")});
    }

    SUBCASE("long message")
    {
        const auto message = std::vector<std::byte>(10'000, std::byte{0x42});

        // we need to start receiving before sending, otherwise we will overflow the queue
        auto received_message_future = server_channel.receive_message();
        REQUIRE_EQ(client_channel.send_message(message), macrosafe::SendResult::Success);

        const auto received_message = received_message_future.get();

        REQUIRE_EQ(received_message.value().size(), message.size());
        REQUIRE_EQ(received_message.value(), message);
    }
}

TEST_CASE("send and receive with encryption")
{
    auto server_channel = macrosafe::EncryptedChannel{
        macrosafe::EncryptedChannel::ServerConfig{.server_port = k_server_port, .client_port = k_client_port}};

    auto client_channel = macrosafe::EncryptedChannel{
        macrosafe::EncryptedChannel::ClientConfig{.server_port = k_client_port, .client_port = k_server_port}};

    SUBCASE("basic")
    {
        auto message_future = server_channel.receive_message();
        REQUIRE_EQ(client_channel.send_message(as_bytes("hello")), macrosafe::SendResult::Success);

        REQUIRE_EQ(message_future.get(), std::optional<std::vector<std::byte>>{as_bytes("hello")});
    }
    SUBCASE("long")
    {
        const auto message = std::vector<std::byte>(10'000, std::byte{0x42});

        // we need to start receiving before sending, otherwise we will overflow the queue
        auto received_message_future = server_channel.receive_message();
        REQUIRE_EQ(client_channel.send_message(message), macrosafe::SendResult::Success);

        const auto received_message = received_message_future.get();

        REQUIRE_EQ(received_message.value().size(), message.size());
        REQUIRE_EQ(received_message.value(), message);
    }
}
