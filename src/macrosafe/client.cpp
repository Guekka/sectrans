#include <client.h>
#include <macrosafe/client.hpp>
#include <macrosafe/common.hpp>
#include <macrosafe/message.hpp>

#include <cstddef>

namespace macrosafe {

auto send_raw_message(std::string message) -> SendResult
{
    if (message.size() > detail::Message::k_max_length)
    {
        return SendResult::Failure;
    }

    // TODO: check if we need a null terminator
    // TODO: check what the return value means
    return sndmsg(message.data(), k_port) == 0 ? SendResult::Success : SendResult::Failure;
}

auto send_message(const detail::Message &message) -> SendResult
{
    auto raw = message.to_raw();

    const auto part_count = raw.size() / detail::Message::k_max_length + 1;

    // TODO: do not saturate the queue if the messages are not being consumed
    for (size_t i = 0; i < part_count; ++i)
    {
        auto part = raw.substr(static_cast<std::string::size_type>(i) * detail::Message::k_max_length,
                               detail::Message::k_max_length);

        if (auto result = send_raw_message(part); result != SendResult::Success)
            return result;
    }

    return SendResult::Success;
}

auto send_message(std::string_view message) -> macrosafe::SendResult
{
    return send_message(detail::Message::from_body(message));
}
} // namespace macrosafe
