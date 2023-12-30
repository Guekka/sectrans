#pragma once

#include <macrosafe/detail/client.hpp>
#include <macrosafe/detail/server.hpp>

#include <future>

namespace macrosafe {

class IChannel
{
public:
    virtual ~IChannel() = default;

    [[nodiscard]] virtual auto receive_message() -> std::future<std::optional<std::vector<std::byte>>> = 0;

    /// @brief Receives a message from the server.
    /// @return The message if successful, otherwise std::nullopt.
    /// @note This function blocks until a message is received.
    [[nodiscard]] virtual auto receive_message_blocking() -> std::optional<std::vector<std::byte>> = 0;

    /// @brief Sends a message to the server.
    /// @param message The message to send.
    /// @return SendResult::Success if successful, otherwise SendResult::Failure.
    [[nodiscard]] virtual auto send_message(std::vector<std::byte> message) -> SendResult = 0;
};

class Channel final : public IChannel
{
    detail::Server server_;
    detail::Client client_;

public:
    Channel(uint16_t server_port, uint16_t client_port);

    /// @copydoc IChannel::receive_message()
    [[nodiscard]] auto receive_message() -> std::future<std::optional<std::vector<std::byte>>> override;

    /// @copydoc IChannel::receive_message_blocking()
    [[nodiscard]] auto receive_message_blocking() -> std::optional<std::vector<std::byte>> override;

    /// @copydoc IChannel::send_message()
    [[nodiscard]] auto send_message(std::vector<std::byte> message) -> SendResult override;
};

} // namespace macrosafe
