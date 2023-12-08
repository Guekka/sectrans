#include <macrosafe/common.hpp>
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

auto Server::receive_message_blocking() const -> std::optional<std::string>
{
    std::string buffer(1024, '\0');
    // TODO: make sure that the C API returns 0 on success
    const int res = getmsg(buffer.data());
    return res == 0 ? std::optional(std::move(buffer)) : std::nullopt;
}
} // namespace macrosafe