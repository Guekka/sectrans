#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <variant>

namespace macrosafe::detail {

enum class MessageType
{
    Header,
    Data,
};

static inline constexpr std::size_t k_max_message_length = 1024;

template<typename Derived>
class MessageBase
{
    static inline const std::string k_header_separator = "|";

protected:
    using CRTP = MessageBase;

    [[nodiscard]] auto derived() const noexcept -> const Derived &
    {
        return static_cast<const Derived &>(*this);
    }

protected:
    [[nodiscard]] virtual auto to_raw_impl() const -> std::string = 0;

public:
    virtual ~MessageBase() = default;

    [[nodiscard]] auto to_raw() const -> std::string
    {
        auto message_id = std::to_string(static_cast<int>(Derived::type()));
        return message_id + k_header_separator + derived().to_raw_impl();
    }

    [[nodiscard]] static auto try_from_raw(std::string_view raw) -> std::optional<Derived>
    {
        auto separator_index = raw.find(k_header_separator);
        if (separator_index == std::string_view::npos)
            return std::nullopt;

        auto message_id_str = raw.substr(0, separator_index);
        try
        {
            auto message_id = std::stoi(std::string(message_id_str));
            if (message_id != static_cast<int>(Derived::type()))
                return std::nullopt;

            return Derived::try_from_raw_impl(raw.substr(separator_index + 1));
        }
        catch (const std::exception &)
        {
            return std::nullopt;
        }
    }
};

class HeaderMessage : public MessageBase<HeaderMessage>
{
    friend CRTP;

    size_t size_;
    size_t part_count_;

    [[nodiscard]] static auto type() -> MessageType { return MessageType::Header; }

    [[nodiscard]] auto to_raw_impl() const -> std::string override;
    [[nodiscard]] static auto try_from_raw_impl(std::string_view raw) -> std::optional<HeaderMessage>;

public:
    HeaderMessage(size_t size, size_t part_count);
    [[nodiscard]] auto size() const noexcept -> size_t { return size_; }
    [[nodiscard]] auto part_count() const noexcept -> size_t { return part_count_; }
};

class DataMessage : public MessageBase<DataMessage>
{
    friend CRTP;

    std::string data_;

    [[nodiscard]] static auto type() -> MessageType { return MessageType::Data; }

    [[nodiscard]] auto to_raw_impl() const -> std::string override;
    [[nodiscard]] static auto try_from_raw_impl(std::string_view raw) -> std::optional<DataMessage>;

public:
    static inline constexpr std::size_t k_max_data_length = 1000; // just to be on the safe side

    DataMessage(std::string data);
    [[nodiscard]] auto data() const noexcept -> const std::string & { return data_; }
};

} // namespace macrosafe::detail
