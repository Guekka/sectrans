#include <macrosafe/message.hpp>

#include <stdexcept>

constexpr std::string_view k_header_separator = "|";

namespace macrosafe::detail {
auto Message::from_raw(std::string_view raw) -> Message
{
    auto message         = Message{};
    auto separator_index = raw.find(k_header_separator);
    if (separator_index == std::string_view::npos)
    {
        // TODO: think about making this a custom exception
        throw std::runtime_error("Invalid message format");
    }
    auto header_length     = raw.substr(0, separator_index);
    message.header_.length = std::stoul(std::string(header_length));

    const auto body_length = std::min(message.header_.length, raw.size() - separator_index - 1);
    message.body_          = raw.substr(separator_index + 1, body_length);

    return message;
}

auto Message::from_body(std::string_view body) -> Message
{
    auto message           = Message{};
    message.body_          = body;
    message.header_.length = body.size();
    return message;
}

auto Message::to_raw() const -> std::string
{
    return std::to_string(header_.length) + std::string(k_header_separator) + std::string(body_);
}

auto Message::get_body() const -> std::string_view
{
    return body_;
}
} // namespace macrosafe::detail