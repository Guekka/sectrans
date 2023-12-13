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

auto Server::receive_message_blocking() const -> std::optional<std::string>
{
    auto raw_header = get_message_raw();

    if (!raw_header)
        return std::nullopt;

    auto header = HeaderMessage::try_from_raw(raw_header.value());
    if (!header)
        return std::nullopt;

    std::string resulting_message;
    for (size_t i = 0; i < header.value().part_count(); ++i)
    {
        auto part = get_message_raw();
        if (!part)
            return std::nullopt;

        auto data = DataMessage::try_from_raw(part.value());
        if (!data)
            return std::nullopt;

        resulting_message += data.value().data();
    }

    if (resulting_message.size() < header.value().size())
        return std::nullopt;

    resulting_message.resize(header.value().size());
    return resulting_message;
}

} // namespace macrosafe::detail
