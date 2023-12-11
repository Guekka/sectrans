#pragma once

#include <string_view>

namespace macrosafe {

enum class SendResult
{
    Success,
    Failure
};

namespace detail {

[[nodiscard]] auto send_message(std::string_view message) -> SendResult;

}
} // namespace macrosafe
