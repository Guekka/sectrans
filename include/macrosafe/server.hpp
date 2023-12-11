#pragma once

#include <optional>
#include <string>

namespace macrosafe {
class Server
{
public:
    Server();
    ~Server();

    [[nodiscard]] auto receive_message_blocking() const -> std::optional<std::string>;
};
} // namespace macrosafe
