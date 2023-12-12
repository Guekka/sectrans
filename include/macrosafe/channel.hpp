#pragma once

#include <macrosafe/detail/client.hpp>
#include <macrosafe/detail/server.hpp>

#include <future>

namespace macrosafe {

class Channel
{
    detail::Server server_;
    detail::Client client_;

public:
    Channel(uint16_t server_port, uint16_t client_port);

    /// @brief Receives a message from the server.
    /// @return The message if successful, otherwise std::nullopt.
    /// @note This function blocks until a message is received.
    [[nodiscard]] auto receive_message() -> std::future<std::optional<std::string>>;

    [[nodiscard]] auto receive_message_blocking() -> std::optional<std::string>;

    /// @brief Sends a message to the server.
    /// @param message The message to send.
    /// @return SendResult::Success if successful, otherwise SendResult::Failure.
    [[nodiscard]] auto send_message(std::string_view message) -> SendResult;
};

} // namespace macrosafe
