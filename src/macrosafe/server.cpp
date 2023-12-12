#include <macrosafe/detail/common.hpp>
#include <macrosafe/detail/message.hpp>
#include <macrosafe/detail/server.hpp>

namespace macrosafe::detail {
Server::Server(uint16_t port)
{
    const int res = lib_.execute<k_start_server_func>(port);
    if (res != 0)
        throw std::runtime_error{"Failed to start server"};
}

Server::~Server()
{
    const int res = lib_.execute<k_stop_server_func>();
    if (res != 0)
        std::cerr << "Failed to stop server" << std::endl; // TODO: another way to handle this?
}

auto Server::get_message_raw() const -> std::optional<std::string>
{
    std::string buffer(1024, '\0');
    const auto res = lib_.execute<k_receive_message_func>(buffer.data());
    // TODO: make sure that the C API returns 0 on success
    if (res != 0)
        return std::nullopt;

    return buffer;
}

auto parse_message(const std::string &message) -> detail::Message
{
    return detail::Message::from_raw(message);
}

auto Server::receive_message_blocking() const -> std::optional<std::string>
{
    auto raw = get_message_raw();

    if (!raw)
        return std::nullopt;

    const auto message = parse_message(raw.value());

    if (message.get_header().length > message.get_body().size())
    {
        const auto remaining_parts = message.get_header().length / detail::Message::k_max_length;

        for (size_t i = 0; i < remaining_parts; ++i) // we already received the first part
        {
            auto part = get_message_raw();
            if (!part)
                return std::nullopt;

            raw.value() += part.value();
        }
    }

    auto final_message = parse_message(raw.value());
    return std::string(final_message.get_body());
}

} // namespace macrosafe::detail
