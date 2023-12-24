// from <https://github.com/tobiaslocker/base64/blob/master/include/base64.hpp>

#ifndef BASE_64_HPP
#define BASE_64_HPP

#include <algorithm>
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
    OutputBuffer encoded;
    encoded.reserve(static_cast<size_t>(1.5 * static_cast<double>(std::distance(begin, end))));
    while (begin != end)
    {
        auto const num_val = static_cast<unsigned char>(*begin);
        offset             = 16 - counter % 3 * 8;
        bit_stream += static_cast<uint32_t>(num_val << offset);
        if (offset == 16)
        {
            encoded.push_back(static_cast<const std::byte>(base64_chars[bit_stream >> 18 & 0x3f]));
        }
        if (offset == 8)
        {
            encoded.push_back(static_cast<const std::byte>(base64_chars[bit_stream >> 12 & 0x3f]));
        }
        if (offset == 0 && counter != 3)
        {
            encoded.push_back(static_cast<const std::byte>(base64_chars[bit_stream >> 6 & 0x3f]));
            encoded.push_back(static_cast<const std::byte>(base64_chars[bit_stream & 0x3f]));
            bit_stream = 0;
        }
        ++counter;
        ++begin;
    }
    if (offset == 16)
    {
        encoded.push_back(static_cast<const std::byte>(base64_chars[bit_stream >> 12 & 0x3f]));
        encoded.push_back(static_cast<const std::byte>('='));
        encoded.push_back(static_cast<const std::byte>('='));
    }
    if (offset == 8)
    {
        encoded.push_back(static_cast<const std::byte>(base64_chars[bit_stream >> 6 & 0x3f]));
        encoded.push_back(static_cast<const std::byte>('='));
    }
    return encoded;
}

inline std::vector<std::byte> to_base64(std::vector<std::byte> data)
{
    return encode_into<std::vector<std::byte>>(std::begin(data), std::end(data));
}

template<class OutputBuffer>
inline OutputBuffer decode_into(std::vector<std::byte> data)
{
    using value_type = typename OutputBuffer::value_type;
    static_assert(std::is_same_v<value_type, char> || std::is_same_v<value_type, unsigned char>
                  || std::is_same_v<value_type, std::byte>);

    size_t counter      = 0;
    uint32_t bit_stream = 0;
    OutputBuffer decoded;
    decoded.reserve(std::size(data));
    for (auto c : data)
    {
        auto const num_val = base64_chars.find(static_cast<char>(c));
        if (num_val != std::string::npos)
        {
            auto const offset = 18 - counter % 4 * 6;
            bit_stream += static_cast<uint32_t>(num_val) << offset;
            if (offset == 12)
            {
                decoded.push_back(static_cast<value_type>(bit_stream >> 16 & 0xff));
            }
            if (offset == 6)
            {
                decoded.push_back(static_cast<value_type>(bit_stream >> 8 & 0xff));
            }
            if (offset == 0 && counter != 4)
            {
                decoded.push_back(static_cast<value_type>(bit_stream & 0xff));
                bit_stream = 0;
            }
        }
        else if (c != std::byte{'='})
        {
            throw std::runtime_error{"Invalid base64 encoded data"};
        }
        counter++;
    }
    return decoded;
}

[[nodiscard]] inline auto from_base64(const std::vector<std::byte> data)
{
    return decode_into<std::vector<std::byte>>(data);
}

} // namespace macrosafe::base64

#endif // BASE_64_HPP
