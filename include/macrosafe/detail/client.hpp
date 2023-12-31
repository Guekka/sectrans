#pragma once

#include <macrosafe/detail/message.hpp>
#include <macrosafe/utils/dylib.hpp>

#include <cassert>
#include <string_view>

namespace macrosafe {

enum class SendResult
{
    Success,
    Failure
};

namespace detail {
constexpr auto k_send_message_func = DyLibFunction<FixedString{"sndmsg"}, int (*)(const char *, uint16_t)>{};

class Client
{
    DyLib<FixedString{"libclient.so"}> lib_;
    uint16_t port_;

    [[nodiscard]] auto send_raw_message(std::vector<std::byte> message) -> SendResult;

public:
    [[nodiscard]] auto send_message(std::span<const std::byte> message) -> SendResult;
    Client(uint16_t port);
};

} // namespace detail
} // namespace macrosafe
