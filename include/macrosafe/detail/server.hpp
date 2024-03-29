#pragma once

#include <macrosafe/utils/dylib.hpp>

#include <optional>
#include <string>

namespace macrosafe::detail {

constexpr auto k_start_server_func    = DyLibFunction<FixedString{"startserver"}, int (*)(uint16_t port)>{};
constexpr auto k_stop_server_func     = DyLibFunction<FixedString{"stopserver"}, int (*)()>{};
constexpr auto k_receive_message_func = DyLibFunction<FixedString{"getmsg"}, int (*)(char buffer[1024])>{};

class Server
{
    DyLib<FixedString{"libserver.so"}> lib_;

    [[nodiscard]] auto get_message_raw() const -> std::optional<std::vector<std::byte>>;

public:
    Server(uint16_t i);
    ~Server();

    [[nodiscard]] auto receive_message_blocking() const -> std::optional<std::vector<std::byte>>;
};
} // namespace macrosafe::detail
