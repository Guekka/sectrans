#include <client.h>
#include <macrosafe/client.hpp>
#include <macrosafe/common.hpp>

#include <string>

namespace macrosafe {
auto send_message(std::string_view message) -> macrosafe::SendResult
{
    // unfortunately, the C API requires a non-const char* here, so we have to copy the string
    std::string message_copy(message);

    const int ret = sndmsg(message_copy.data(), k_port);

    // TODO: make sure that the C API returns 0 on success
    return ret == 0 ? macrosafe::SendResult::Success : macrosafe::SendResult::Failure;
}
} // namespace macrosafe