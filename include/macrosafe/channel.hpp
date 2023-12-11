#pragma once

#include <macrosafe/detail/client.hpp>
#include <macrosafe/detail/server.hpp>

#include <future>

namespace macrosafe {

class Channel
{
    static inline detail::Server server_;

public:
    /// @brief Receives a message from the server.
    /// @return The message if successful, otherwise std::nullopt.
    /// @note This function blocks until a message is received.
    [[nodiscard]] static auto receive_message() -> std::future<std::optional<std::string>>;

    [[nodiscard]] static auto receive_message_blocking() -> std::optional<std::string>;

    /// @brief Sends a message to the server.
    /// @param message The message to send.
    /// @return SendResult::Success if successful, otherwise SendResult::Failure.
    [[nodiscard]] static auto send_message(std::string_view message) -> SendResult;
};

} // namespace macrosafe
