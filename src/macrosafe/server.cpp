#include <macrosafe/detail/common.hpp>
#include <macrosafe/detail/message.hpp>
#include <macrosafe/detail/server.hpp>
#include <server.h>

namespace macrosafe::detail {
Server::Server()
{
    startserver(k_port);
}

Server::~Server()
{
    stopserver();
}

auto parse_message(const std::string &message) -> detail::Message
{
    return detail::Message::from_raw(message);
}

auto Server::receive_message_blocking() const -> std::optional<std::string>
{
    std::string buffer(1024, '\0');
    // TODO: make sure that the C API returns 0 on success
    int res = getmsg(buffer.data());
    if (res != 0)
        return std::nullopt;

    const auto message = parse_message(buffer);

    if (message.get_header().length > message.get_body().size())
    {
        const auto remaining_parts = message.get_header().length / detail::Message::k_max_length;

        for (size_t i = 0; i < remaining_parts; ++i) // we already received the first part
        {
            std::string part(1024, '\0');
            res = getmsg(part.data());
            if (res != 0)
                return std::nullopt;

            buffer += part;
        }
    }

    auto final_message = parse_message(buffer);
    return std::string(final_message.get_body());
}

} // namespace macrosafe::detail
