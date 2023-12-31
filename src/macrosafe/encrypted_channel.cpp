
#include <macrosafe/encrypted_channel.hpp>

#include <array>

namespace macrosafe {

void EncryptedChannel::init_client()
{
    detail::CryptoSessionHelperClient helper;

    auto initial_packet = helper.initial_packet();

    if (channel_.send_message(initial_packet) != SendResult::Success)
        throw std::runtime_error{"Failed to send initial packet."};

    auto response = channel_.receive_message_blocking();
    if (!response)
        throw std::runtime_error{"Failed to receive initial packet."};

    auto final_packet = helper.final_packet(response.value());
    crypto_session_   = final_packet.session;

    if (channel_.send_message(final_packet.packet) != SendResult::Success)
        throw std::runtime_error{"Failed to verify initial packet."};
}

void EncryptedChannel::init_server()
{
    detail::CryptoSessionHelperServer helper;

    auto initial_packet = [this] {
        while (true)
        {
            auto ret = channel_.receive_message_blocking();
            if (!ret)
                throw std::runtime_error{"Failed to receive initial packet."};

            if (ret.value() == k_renegociate_message)
                continue;

            return ret.value();
        }
    }();

    auto followup = helper.process_initial(initial_packet);

    if (channel_.send_message(followup) != SendResult::Success)
        throw std::runtime_error{"Failed to send initial packet."};

    auto final_packet = channel_.receive_message_blocking();
    if (!final_packet)
        throw std::runtime_error{"Failed to receive final packet."};

    crypto_session_ = helper.process_final(final_packet.value());
}

EncryptedChannel::EncryptedChannel(ServerConfig config)
    : channel_(config.server_port, config.client_port)
    , mode_(Mode::Server)
{
}

EncryptedChannel::EncryptedChannel(ClientConfig config)
    : channel_(config.server_port, config.client_port)
    , mode_(Mode::Client)
{
}

void EncryptedChannel::handle_renogociate()
{
    switch (mode_)
    {
        case Mode::Client: init_client(); break;
        case Mode::Server: init_server(); break;
    }
}

auto macrosafe::EncryptedChannel::receive_message() -> std::future<std::optional<std::vector<std::byte>>>
{
    return std::async(std::launch::async, [this] { return receive_message_blocking(); });
}

auto EncryptedChannel::receive_message_blocking() -> std::optional<std::vector<std::byte>>
{
    auto message = [this] {
        while (true)
        {
            auto ret = channel_.receive_message_blocking();
            if (!ret)
                return std::optional<std::vector<std::byte>>{};

            if (ret.value() == k_renegociate_message)
            {
                handle_renogociate();
                continue;
            }

            return ret;
        }
    }();

    assert_initialized();
    return crypto_session_->decrypt(message.value());
}

auto EncryptedChannel::send_message(std::span<const std::byte> message) -> SendResult
{
    if (!crypto_session_)
    {
        if (channel_.send_message(k_renegociate_message) != SendResult::Success)
            throw std::runtime_error{"Failed to init connection."};
        init_client();
    }

    assert_initialized();
    auto encrypted = crypto_session_->encrypt(message);
    return channel_.send_message(std::move(encrypted));
}

void EncryptedChannel::assert_initialized()
{
    if (!crypto_session_)
        throw std::runtime_error{"EncryptedChannel not initialized."};
}
} // namespace macrosafe
