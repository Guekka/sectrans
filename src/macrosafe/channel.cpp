#include <macrosafe/channel.hpp>

namespace macrosafe {

Channel::Channel(uint16_t server_port, uint16_t client_port)
    : server_{server_port}
    , client_{client_port}
{
}

auto Channel::receive_message() -> std::future<std::optional<std::string>>
{
    return std::async(std::launch::async, [this]() { return receive_message_blocking(); });
}

auto Channel::receive_message_blocking() -> std::optional<std::string>
{
    return server_.receive_message_blocking();
}

auto Channel::send_message(std::string_view message) -> SendResult
{
    return client_.send_message(message);
}

} // namespace macrosafe
