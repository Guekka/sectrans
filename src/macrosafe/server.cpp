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

auto Server::get_message_raw() const -> std::optional<std::vector<std::byte>>
{
    std::vector<std::byte> buffer(k_max_message_length);
    char *as_char  = std::launder(reinterpret_cast<char *>(buffer.data()));
    const auto res = lib_.execute<k_receive_message_func>(as_char);
    // TODO: make sure that the C API returns 0 on success
    if (res != 0)
        return std::nullopt;

    return buffer;
}

auto Server::receive_message_blocking() const -> std::optional<std::vector<std::byte>>
{
    auto raw = get_message_raw();
    if (!raw)
        return std::nullopt;

    auto message = MessagePart::try_from_raw(raw.value());
    if (!message)
        return std::nullopt;

    if (message.value().header().part_index != 0)
        return std::nullopt;

    std::vector<std::byte> resulting_message;
    resulting_message.reserve(message.value().header().total_size);
    resulting_message.insert(resulting_message.end(),
                             message.value().payload().begin(),
                             message.value().payload().end());

    for (size_t i = 1; i < message.value().header().part_count; ++i)
    {
        auto part = get_message_raw();
        if (!part)
            return std::nullopt;

        auto parsed_part = MessagePart::try_from_raw(part.value());
        if (!parsed_part)
            return std::nullopt;

        if (parsed_part.value().header().part_index != i)
            return std::nullopt;

        resulting_message.insert(resulting_message.end(),
                                 parsed_part.value().payload().begin(),
                                 parsed_part.value().payload().end());
    }

    if (resulting_message.size() != message.value().header().total_size)
        return std::nullopt;

    return resulting_message;
}

} // namespace macrosafe::detail
