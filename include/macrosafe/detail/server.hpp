#pragma once

#include <macrosafe/utils/dylib.hpp>

#include <optional>
#include <string>

namespace macrosafe::detail {

constexpr auto k_start_server_func    = DyLibFunction<FixedString{"startserver"}, void (*)(uint16_t port)>{};
constexpr auto k_stop_server_func     = DyLibFunction<FixedString{"stopserver"}, void (*)()>{};
constexpr auto k_receive_message_func = DyLibFunction<FixedString{"getmsg"}, int (*)(char buffer[1024])>{};

class Server
{
    DyLib<FixedString{"libserver.so"}> lib_;

    [[nodiscard]] auto get_message_raw() const -> std::optional<std::string>;

public:
    Server();
    ~Server();

    [[nodiscard]] auto receive_message_blocking() const -> std::optional<std::string>;
};
} // namespace macrosafe::detail
