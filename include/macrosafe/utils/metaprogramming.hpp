#pragma once

#include <cstddef>
namespace macrosafe {
template<typename... Ts>
struct Overload : Ts...
{
    using Ts::operator()...;
};

template<typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

template<size_t Length>
struct FixedString
{
    char chars[Length + 1] = {}; // +1 for null terminator
};

template<size_t N>
FixedString(const char (&arr)[N]) -> FixedString<N - 1>; // Drop the null terminator

} // namespace macrosafe
