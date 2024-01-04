#pragma once

#include <macrosafe/utils/base64.hpp>
#include <macrosafe/utils/crypto.hpp>

#include <filesystem>
#include <string>
#include <vector>

class Password
{
    std::vector<std::byte> hashed_password_;

    Password() = default;
    [[nodiscard]] static auto hash(std::string password) -> std::vector<std::byte>;

public:
    explicit Password(std::string password);
    [[nodiscard]] auto check(std::string password) const -> bool;
};

struct User
{
    std::string username;
    Password password;
};

class Auth
{
    std::vector<User> users_;

public:
    explicit Auth(std::vector<User> users);

    [[nodiscard]] auto check(std::string username, std::string password) const -> bool;

    [[nodiscard]] auto register_user(std::string username, std::string password) -> bool;
};
