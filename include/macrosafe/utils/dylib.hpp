#pragma once

#include "metaprogramming.hpp"

#include <dlfcn.h>

#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace macrosafe {

static auto get_dlerror() -> std::string
{
    static std::mutex dlerror_mutex; // dlerror is not thread-safe
    auto lock = std::lock_guard(dlerror_mutex);
    return dlerror(); // NOLINT(concurrency-mt-unsafe)
}

class DyLibError : public std::runtime_error
{
public:
    enum class ErrorType
    {
        Open,
        Symbol
    };

    explicit DyLibError(const std::string &message, ErrorType type)
        : std::runtime_error(message)
        , type(type)
    {
    }

    [[nodiscard]] auto get_type() const noexcept -> ErrorType { return type; }

private:
    ErrorType type;
};

template<FixedString Name, class FuncPtr>
struct DyLibFunction
{
    static constexpr auto name = Name;
    using type                 = FuncPtr;
};

template<FixedString lib_name>
class DyLib
{
    static constexpr auto deleter = [](void *handle) noexcept { dlclose(handle); };
    std::unique_ptr<void, decltype(deleter)> handle_;

public:
    DyLib()
        : handle_(dlopen(lib_name.chars, RTLD_LAZY))
    {
        if (handle_ == nullptr)
            throw DyLibError(get_dlerror(), DyLibError::ErrorType::Open);
    }

    template<DyLibFunction func, typename... FuncArgs>
    [[nodiscard]] constexpr auto execute(FuncArgs... args) const -> decltype(auto)
    {
        auto *f = reinterpret_cast<decltype(func)::type>(dlsym(handle_.get(), func.name.chars));
        if (f == nullptr)
            throw DyLibError(get_dlerror(), DyLibError::ErrorType::Symbol);

        return (*f)(args...);
    }
};
} // namespace macrosafe
