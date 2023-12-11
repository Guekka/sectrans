#pragma once

#include <optional>
#include <string>

namespace macrosafe::detail {
class Server
{
public:
    Server();
    ~Server();

    [[nodiscard]] auto receive_message_blocking() const -> std::optional<std::string>;
};
} // namespace macrosafe::detail
