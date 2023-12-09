#pragma once

#include <cstddef>
#include <string>
namespace macrosafe::detail {

class Message
{
private:
    struct Header
    {
        size_t length;
    } header_;

    std::string body_;

public:
    static constexpr auto k_max_length = 1024;

    [[nodiscard]] static auto from_raw(std::string_view raw) -> Message;
    [[nodiscard]] static auto from_body(std::string_view body) -> Message;

    [[nodiscard]] auto to_raw() const -> std::string;

    [[nodiscard]] auto get_body() const -> std::string_view;
    [[nodiscard]] auto get_header() const -> Header;
};
} // namespace macrosafe::detail