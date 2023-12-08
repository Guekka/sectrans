#include <macrosafe/common.hpp>
#include <macrosafe/message.hpp>
#include <macrosafe/server.hpp>
#include <server.h>

namespace macrosafe {
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
    const int res = getmsg(buffer.data());

    const auto message = parse_message(buffer);
    // TODO: handle header.length != body.size()

    return res == 0 ? std::optional(std::string(message.get_body())) : std::nullopt;
}
} // namespace macrosafe