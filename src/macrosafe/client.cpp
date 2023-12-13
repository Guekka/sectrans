
#include <macrosafe/detail/client.hpp>
#include <macrosafe/detail/common.hpp>
#include <macrosafe/detail/message.hpp>

namespace macrosafe::detail {

Client::Client(uint16_t port)
    : port_{port}
{
}

auto Client::send_raw_message(std::string message) -> SendResult
{
    if (message.size() > detail::k_max_message_length)
        return SendResult::Failure;

    // TODO: check if we need a null terminator
    // TODO: check what the return value means
    return lib_.execute<k_send_message_func>(message.data(), port_) == 0 ? SendResult::Success
                                                                         : SendResult::Failure;
}

auto Client::send_message(std::string_view message) -> SendResult
{
    const auto part_count = message.size() / detail::DataMessage::k_max_data_length
                            + (message.size() % detail::DataMessage::k_max_data_length != 0 ? 1 : 0);

    // first send the header
    const auto header = detail::HeaderMessage{message.size(), part_count};
    if (auto result = send_message(header); result != SendResult::Success)
        return result;

    // TODO: do not saturate the queue if the messages are not being consumed
    for (size_t i = 0; i < part_count; ++i)
    {
        auto part = message.substr(static_cast<std::string::size_type>(i)
                                       * detail::DataMessage::k_max_data_length,
                                   detail::DataMessage::k_max_data_length);

        if (auto result = send_message(detail::DataMessage{std::string(part)}); result != SendResult::Success)
            return result;
    }

    return SendResult::Success;
}

} // namespace macrosafe::detail
