#pragma once

#include <bit>
#include <cstring>
#include <optional>
#include <vector>

namespace macrosafe {

namespace endian {

#define MACROSAFE_BSWAP16 __builtin_bswap16
#define MACROSAFE_BSWAP32 __builtin_bswap32
#define MACROSAFE_BSWAP64 __builtin_bswap64

/// \brief Reverses the endian format of a given input.
///
/// \param value The value to reverse.
/// \return The reversed value.
template<class T>
[[nodiscard]] auto reverse(T value) noexcept -> T
{
    if constexpr (sizeof(T) == 1)
        return static_cast<T>(value);

    if constexpr (sizeof(T) == 2)
        return static_cast<T>(MACROSAFE_BSWAP16(value));

    if constexpr (sizeof(T) == 4)
        return static_cast<T>(MACROSAFE_BSWAP32(value));

    if constexpr (sizeof(T) == 8)
        return static_cast<T>(MACROSAFE_BSWAP64(value));

    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
                  "reverse() is only implemented for 1, 2, 4, and 8 byte types.");
}

/// \brief Loads the given type from the given buffer, with the given endian format,
///		into the native endian format.
///
/// \param src The buffer to load from.
/// \return The value loaded from the given buffer.
template<std::endian E, class T>
[[nodiscard]] auto load(std::vector<std::byte> src) noexcept -> T
{
    alignas(T) std::byte buf[sizeof(T)] = {};
    std::memcpy(buf, src.data(), sizeof(T));
    const auto val = *std::launder(reinterpret_cast<const T *>(buf));
    if constexpr (std::endian::native != E)
        return reverse(val);

    return val;
}

/// \brief Stores the given type into the given buffer, from the native endian format
///		into the given endian format.
///
/// \param value The value to be stored.
template<std::endian E, class T>
auto store(T value) noexcept -> std::vector<std::byte>
{
    if constexpr (std::endian::native != E)
        value = reverse(value);

    std::vector<std::byte> dst(sizeof(T));
    std::memcpy(dst.data(), &value, sizeof(T));
    return dst;
}
} // namespace endian

constexpr auto k_endian = std::endian::big;

template<class T>
[[nodiscard]] auto serialize(T value) noexcept -> std::vector<std::byte>
{
    return endian::store<k_endian>(value);
}

template<class T>
void serialize_concat(std::vector<std::byte> &dst, T value) noexcept
{
    const auto serialized = serialize(value);
    dst.insert(dst.end(), serialized.begin(), serialized.end());
}

template<class T>
[[nodiscard]] auto deserialize(std::vector<std::byte> src) noexcept -> T
{
    return endian::load<k_endian, T>(src);
}

template<class... T>
[[nodiscard]] auto serialize(T... values) noexcept -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
    (serialize_concat(result, values), ...);
    return result;
}

template<class T>
[[nodiscard]] auto deserialize_advance(std::span<const std::byte> &src) noexcept -> std::decay_t<T>
{
    auto result = deserialize<std::decay_t<T>>(std::vector<std::byte>(src.begin(), src.begin() + sizeof(T)));
    src         = src.subspan(sizeof(T));
    return result;
}

template<class... T>
[[nodiscard]] auto deserialize(std::span<const std::byte> src, T &...values) noexcept -> bool
{
    auto total_size = 0;
    ((total_size += sizeof(decltype(values))), ...);
    if (src.size() < total_size)
        return false;

    ((values = deserialize_advance<decltype(values)>(src)), ...);
    return true;
}

} // namespace macrosafe
