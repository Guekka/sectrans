#pragma once

#include <string_view>

namespace macrosafe {
enum class SendResult
{
    Success,
    Failure
};

[[nodiscard]] auto send_message(std::string_view message) -> SendResult;
} // namespace macrosafe
