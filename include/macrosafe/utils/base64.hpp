// from <https://github.com/tobiaslocker/base64/blob/master/include/base64.hpp>

#ifndef BASE_64_HPP
#define BASE_64_HPP

#include <algorithm>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace macrosafe::base64 {

inline constexpr std::string_view base64_chars{"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                               "abcdefghijklmnopqrstuvwxyz"
                                               "0123456789+/"};

template<class OutputBuffer, class InputIterator>
inline auto encode_into(InputIterator begin, InputIterator end) -> OutputBuffer
{
    static_assert(std::is_same_v<std::decay_t<decltype(*begin)>, char>
                  || std::is_same_v<std::decay_t<decltype(*begin)>, unsigned char>
                  || std::is_same_v<std::decay_t<decltype(*begin)>, std::byte>);

    size_t counter      = 0;
    uint32_t bit_stream = 0;
    size_t offset       = 0;
    size_t index        = 0;
    OutputBuffer encoded;
    encoded.resize(1.5 * std::distance(begin, end));
    while (begin != end)
    {
        auto const num_val = static_cast<unsigned char>(*begin);
        offset             = 16 - counter % 3 * 8;
        bit_stream += static_cast<uint32_t>(num_val << offset);
        if (offset == 16)
        {
            encoded[index++] = static_cast<const std::byte>(base64_chars[bit_stream >> 18 & 0x3f]);
        }
        if (offset == 8)
        {
            encoded[index++] = static_cast<const std::byte>(base64_chars[bit_stream >> 12 & 0x3f]);
        }
        if (offset == 0 && counter != 3)
        {
            encoded[index++] = static_cast<const std::byte>(base64_chars[bit_stream >> 6 & 0x3f]);
            encoded[index++] = static_cast<const std::byte>(base64_chars[bit_stream & 0x3f]);
            bit_stream       = 0;
        }
        ++counter;
        ++begin;
    }
    if (offset == 16)
    {
        encoded[index++] = static_cast<const std::byte>(base64_chars[bit_stream >> 12 & 0x3f]);
        encoded[index++] = static_cast<const std::byte>('=');
        encoded[index++] = static_cast<const std::byte>('=');
    }
    if (offset == 8)
    {
        encoded[index++] = static_cast<const std::byte>(base64_chars[bit_stream >> 6 & 0x3f]);
        encoded[index++] = static_cast<const std::byte>('=');
    }
    encoded.erase(encoded.begin() + index, encoded.end());
    return encoded;
}

inline std::vector<std::byte> to_base64(std::span<const std::byte> data)
{
    return encode_into<std::vector<std::byte>>(std::begin(data), std::end(data));
}

template<class OutputBuffer>
inline OutputBuffer decode_into(std::span<const std::byte> data)
{
    using value_type = typename OutputBuffer::value_type;
    static_assert(std::is_same_v<value_type, char> || std::is_same_v<value_type, unsigned char>
                  || std::is_same_v<value_type, std::byte>);

    size_t counter      = 0;
    size_t index        = 0;
    uint32_t bit_stream = 0;
    OutputBuffer decoded;
    decoded.resize(std::size(data));
    for (auto c : data)
    {
        auto const num_val = base64_chars.find(static_cast<char>(c));
        if (num_val != std::string::npos)
        {
            auto const offset = 18 - counter % 4 * 6;
            bit_stream += static_cast<uint32_t>(num_val) << offset;
            if (offset == 12)
            {
                decoded[index++] = static_cast<value_type>(bit_stream >> 16 & 0xff);
            }
            if (offset == 6)
            {
                decoded[index++] = static_cast<value_type>(bit_stream >> 8 & 0xff);
            }
            if (offset == 0 && counter != 4)
            {
                decoded[index++] = static_cast<value_type>(bit_stream & 0xff);
                bit_stream       = 0;
            }
        }
        else if (c != std::byte{'='})
        {
            throw std::runtime_error{"Invalid base64 encoded data"};
        }
        counter++;
    }
    decoded.erase(decoded.begin() + index, decoded.end());
    return decoded;
}

[[nodiscard]] inline auto from_base64(const std::span<const std::byte> data)
{
    return decode_into<std::vector<std::byte>>(data);
}

[[nodiscard]] constexpr size_t max_data_size_for_encoded_size(size_t encoded_size)
{
    return encoded_size / 4 * 3 - 4;
}

} // namespace macrosafe::base64

#endif // BASE_64_HPP
