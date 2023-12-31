
#include <macrosafe/encrypted_channel.hpp>

#include <array>

namespace macrosafe {

void EncryptedChannel::init_client()
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

void EncryptedChannel::init_server()
{
    detail::CryptoSessionHelperServer helper;

    std::cout << "Server waiting for initial packet\n" << std::flush;

    auto initial_packet = [this] {
        while (true)
        {
            auto ret = channel_.receive_message_blocking();
            if (!ret)
                throw std::runtime_error{"Failed to receive initial packet."};

            if (ret.value() == k_renegociate_message)
            {
                std::cout << "Server received handle_renogociate message while already renegociating. "
                             "Ignoring.\n"
                          << std::flush;
                continue;
            }

            return ret.value();
        }
    }();

    std::cout << "Server received initial packet[";
    for (auto byte : initial_packet)
        std::cout << static_cast<int>(byte) << ", ";
    std::cout << "]\n" << std::flush;

    auto followup = helper.process_initial(initial_packet);

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
    std::cout << "Renegociating connection\n" << std::flush;
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

auto EncryptedChannel::send_message(std::vector<std::byte> message) -> SendResult
{
    if (!crypto_session_)
    {
        if (channel_.send_message(k_renegociate_message) != SendResult::Success)
            throw std::runtime_error{"Failed to init connection."};
        init_client();
    }

    assert_initialized();
    auto encrypted = crypto_session_->encrypt(std::move(message));
    return channel_.send_message(std::move(encrypted));
}

void EncryptedChannel::assert_initialized()
{
    if (!crypto_session_)
        throw std::runtime_error{"EncryptedChannel not initialized."};
}
} // namespace macrosafe
