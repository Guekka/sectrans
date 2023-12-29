
#include <macrosafe/encrypted_channel.hpp>

#include <array>

namespace macrosafe {

EncryptedChannel::EncryptedChannel(ClientConfig config)
    : channel_(config.server_port, config.client_port)
{
    detail::CryptoSessionHelperClient helper;

    auto initial_packet = helper.initial_packet();
    std::cout << "Client sending initial packet: [";
    for (auto byte : initial_packet)
        std::cout << static_cast<int>(byte) << ", ";
    std::cout << "]\n" << std::flush;

    if (channel_.send_message(initial_packet) != SendResult::Success)
        throw std::runtime_error{"Failed to send initial packet."};

    std::cout << "Client sent initial packet\n" << std::flush;

    auto response = channel_.receive_message_blocking();
    if (!response)
        throw std::runtime_error{"Failed to receive initial packet."};

    std::cout << "Client received initial packet\n" << std::flush;

    auto final_packet = helper.final_packet(response.value());
    crypto_session_   = final_packet.session;

    if (channel_.send_message(final_packet.packet) != SendResult::Success)
        throw std::runtime_error{"Failed to verify initial packet."};

    std::cout << "Client sent final packet\n" << std::flush;
}

EncryptedChannel::EncryptedChannel(ServerConfig config)
    : channel_(config.server_port, config.client_port)
{
    detail::CryptoSessionHelperServer helper;

    std::cout << "Server waiting for initial packet\n" << std::flush;

    auto initial_packet = channel_.receive_message_blocking();
    if (!initial_packet)
        throw std::runtime_error{"Failed to receive initial packet."};

    std::cout << "Server received initial packet[";
    for (auto byte : initial_packet.value())
        std::cout << static_cast<int>(byte) << ", ";
    std::cout << "]\n" << std::flush;

    auto followup = helper.process_initial(initial_packet.value());

    std::cout << "Server sending followup packet\n" << std::flush;

    if (channel_.send_message(followup) != SendResult::Success)
        throw std::runtime_error{"Failed to send initial packet."};

    std::cout << "Server sent initial packet\n" << std::flush;

    auto final_packet = channel_.receive_message_blocking();
    if (!final_packet)
        throw std::runtime_error{"Failed to receive final packet."};

    std::cout << "Server received final packet\n" << std::flush;

    crypto_session_ = helper.process_final(final_packet.value());
}

auto macrosafe::EncryptedChannel::receive_message() -> std::future<std::optional<std::vector<std::byte>>>
{
    auto message = channel_.receive_message();
    return std::async(std::launch::async,
                      [this, message = std::move(message)]() mutable -> std::optional<std::vector<std::byte>> {
                          auto result = message.get();
                          if (!result)
                              return std::nullopt;

                          return crypto_session_.decrypt(result.value());
                      });
}

auto EncryptedChannel::receive_message_blocking() -> std::optional<std::vector<std::byte>>
{
    auto message = channel_.receive_message_blocking();
    if (!message)
        return std::nullopt;

    return crypto_session_.decrypt(message.value());
}

auto EncryptedChannel::send_message(std::vector<std::byte> message) -> SendResult
{
    auto encrypted = crypto_session_.encrypt(std::move(message));
    return channel_.send_message(std::move(encrypted));
}
} // namespace macrosafe
