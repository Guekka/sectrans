#pragma once

#include <macrosafe/channel.hpp>
#include <macrosafe/utils/crypto.hpp>

namespace macrosafe {

class EncryptedChannel final : public IChannel
{
    Channel channel_;

    detail::CryptoSession crypto_session_;

public:
    struct ClientConfig
    {
        uint16_t server_port;
        uint16_t client_port;
    };

    struct ServerConfig
    {
        uint16_t server_port;
        uint16_t client_port;
    };

    EncryptedChannel(EncryptedChannel &&) noexcept = default;

    EncryptedChannel(ClientConfig config);
    EncryptedChannel(ServerConfig config);

    /// @copydoc Channel::receive_message()
    [[nodiscard]] auto receive_message() -> std::future<std::optional<std::vector<std::byte>>> override;

    /// @copydoc Channel::receive_message_blocking()
    [[nodiscard]] auto receive_message_blocking() -> std::optional<std::vector<std::byte>> override;

    /// @copydoc Channel::send_message()
    [[nodiscard]] auto send_message(std::vector<std::byte> message) -> SendResult override;
};

} // namespace macrosafe
