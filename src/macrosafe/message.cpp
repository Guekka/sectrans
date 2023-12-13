#include <macrosafe/detail/message.hpp>

#include <stdexcept>

namespace macrosafe::detail {

static inline const std::string k_header_message_inline_separator = ";";

HeaderMessage::HeaderMessage(size_t size, size_t part_count)
    : size_{size}
    , part_count_{part_count}
{
}

auto HeaderMessage::to_raw_impl() const -> std::string
{
    return std::to_string(size_) + k_header_message_inline_separator + std::to_string(part_count_);
}

auto HeaderMessage::try_from_raw_impl(std::string_view raw) -> std::optional<HeaderMessage>
{
    try
    {
        auto separator_index = raw.find(k_header_message_inline_separator);
        if (separator_index == std::string_view::npos)
            return std::nullopt;

        auto size_str       = raw.substr(0, separator_index);
        auto part_count_str = raw.substr(separator_index + 1);

        auto size       = std::stoull(std::string(size_str));
        auto part_count = std::stoull(std::string(part_count_str));

        return std::optional{HeaderMessage{size, part_count}};
    }
    catch (const std::exception &)
    {
        return std::nullopt;
    }
}

DataMessage::DataMessage(std::string data)
    : data_{std::move(data)}
{
}

auto DataMessage::to_raw_impl() const -> std::string
{
    return data_;
}

auto DataMessage::try_from_raw_impl(std::string_view raw) -> std::optional<DataMessage>
{
    auto payload = raw.substr(0, k_max_data_length);
    return std::optional{DataMessage{std::string(payload)}};
}

} // namespace macrosafe::detail
