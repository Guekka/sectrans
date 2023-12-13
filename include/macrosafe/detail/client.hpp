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

    [[nodiscard]] auto send_raw_message(std::string message) -> SendResult;

    template<class Derived>
    [[nodiscard]] auto send_message(const detail::MessageBase<Derived> &message) -> SendResult
    {
        return send_raw_message(message.to_raw());
    }

public:
    [[nodiscard]] auto send_message(std::string_view message) -> SendResult;
    Client(uint16_t port);
};

} // namespace detail
} // namespace macrosafe
