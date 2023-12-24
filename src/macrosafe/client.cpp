
#include <macrosafe/detail/client.hpp>
#include <macrosafe/detail/common.hpp>
#include <macrosafe/detail/message.hpp>

namespace macrosafe::detail {

Client::Client(uint16_t port)
    : port_{port}
{
}

auto Client::send_raw_message(std::vector<std::byte> message) -> SendResult
{
    if (message.size() > detail::k_max_message_length)
        return SendResult::Failure;

    message.push_back(std::byte{0}); // the C function expects a null-terminated string
    const char *as_char = std::launder(reinterpret_cast<char *>(message.data()));

    return lib_.execute<k_send_message_func>(as_char, port_) == 0 ? SendResult::Success : SendResult::Failure;
}

auto Client::send_message(std::vector<std::byte> message) -> SendResult
{
    const size_t computed_part_count = message.size() / k_max_data_length
                                       + (message.size() % k_max_data_length != 0 ? 1 : 0);
    const size_t part_count = std::max(computed_part_count, static_cast<size_t>(1)); // allow empty messages

    // TODO: do not saturate the queue if the messages are not being consumed
    for (size_t i = 0; i < part_count; ++i)
    {
        auto part = std::vector<std::byte>{};
        part.insert(part.end(),
                    message.begin() + static_cast<int64_t>(i * k_max_data_length),
                    message.begin()
                        + static_cast<int64_t>(std::min((i + 1) * k_max_data_length, message.size())));

        const auto header = Header{
            .total_size = message.size(),
            .part_size  = static_cast<uint16_t>(part.size()),
            .part_count = static_cast<uint16_t>(part_count),
            .part_index = static_cast<uint16_t>(i),
        };

        const auto result = send_raw_message(MessagePart{header, part}.to_raw());
        if (result != SendResult::Success)
            return result;
    }
    return SendResult::Success;
}

auto Client::send_message(const MessagePart &message) -> SendResult
{
    return send_raw_message(message.to_raw());
}

} // namespace macrosafe::detail
