#pragma once

#include <macrosafe/utils/base64.hpp>
#include <macrosafe/utils/serde.hpp>

#include <cstddef>
#include <cstring>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace macrosafe::detail {

struct Header
{
    size_t total_size;
    uint16_t part_size;
    uint16_t part_count;
    uint16_t part_index;
};

constexpr auto k_packed_header_size = sizeof(size_t) + 3 * sizeof(uint16_t);

constexpr size_t k_max_message_length = 1024;
// account for base64 encoding
constexpr size_t k_max_data_length    = base64::max_data_size_for_encoded_size(k_max_message_length - k_packed_header_size);

class MessagePart
{
    Header header_;
    std::vector<std::byte> payload_;

public:
    MessagePart(Header header, std::vector<std::byte> payload)
        : header_{header}
        , payload_{std::move(payload)}
    {
    }

    [[nodiscard]] auto to_raw() const -> std::vector<std::byte>
    {
        std::vector<std::byte> result = serialize(header_.total_size,
                                                  header_.part_size,
                                                  header_.part_count,
                                                  header_.part_index);

        result.insert(result.end(), payload_.begin(), payload_.end());

        return base64::to_base64(result);
    }

    [[nodiscard]] static auto try_from_raw(std::vector<std::byte> raw) -> std::optional<MessagePart>
    {
        if (raw.size() < k_packed_header_size)
            return std::nullopt;

        if (raw.size() > k_max_message_length)
            return std::nullopt;

        auto end = std::find(raw.begin(), raw.end(), std::byte{0});
        if (end != raw.end())
            raw.erase(end, raw.end());

        raw = base64::from_base64(raw);

        Header header{};
        deserialize(raw, header.total_size, header.part_size, header.part_count, header.part_index);

        if (raw.size() < k_packed_header_size + header.part_size)
            return std::nullopt;

        raw.erase(raw.begin(), raw.begin() + k_packed_header_size);
        raw.resize(header.part_size);

        return MessagePart{header, std::move(raw)};
    }

    [[nodiscard]] auto header() const -> const Header & { return header_; }

    [[nodiscard]] auto payload() const -> const std::vector<std::byte> & { return payload_; }
};

} // namespace macrosafe::detail
