#include <macrosafe/channel.hpp>

namespace macrosafe {

auto Channel::receive_message() -> std::future<std::optional<std::string>>
{
    return std::async(std::launch::async, receive_message_blocking);
}

auto Channel::receive_message_blocking() -> std::optional<std::string>
{
    return server_.receive_message_blocking();
}

auto Channel::send_message(std::string_view message) -> SendResult
{
    return detail::send_message(message);
}

} // namespace macrosafe
