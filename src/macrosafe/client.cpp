#include <client.h>
#include <macrosafe/client.hpp>
#include <macrosafe/common.hpp>
#include <macrosafe/message.hpp>

namespace macrosafe {

auto send_message(const detail::Message &message) -> SendResult
{
    auto raw = message.to_raw();

    const int ret = sndmsg(raw.data(), k_port);

    // TODO: make sure that the C API returns 0 on success
    return ret == 0 ? macrosafe::SendResult::Success : macrosafe::SendResult::Failure;
}

auto send_message(std::string_view message) -> macrosafe::SendResult
{
    return send_message(detail::Message::from_body(message));
}
} // namespace macrosafe