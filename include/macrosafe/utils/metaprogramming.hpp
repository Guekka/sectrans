#pragma once

namespace macrosafe {
template<typename... Ts>
struct Overload : Ts...
{
    using Ts::operator()...;
};

template<typename... Ts>
Overload(Ts...) -> Overload<Ts...>;

} // namespace macrosafe
