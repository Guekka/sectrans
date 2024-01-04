#include "auth.hpp"

#include <macrosafe/utils/base64.hpp>

#include <algorithm>
#include <numeric>

static inline const macrosafe::PasswordHasher k_password_hasher{};

Password::Password(std::string password)
    : hashed_password_{hash(std::move(password))}
{
}

auto Password::hash(std::string password) -> std::vector<std::byte>
{
    return k_password_hasher.hash(std::move(password));
}

auto Password::check(std::string password) const -> bool
{
    return k_password_hasher.verify(std::move(password), hashed_password_);
}

Auth::Auth(std::vector<User> users)
    : users_{std::move(users)}
{
}

auto Auth::check(std::string username, std::string password) const -> bool
{
    return std::ranges::any_of(users_, [&](const auto &user) {
        return user.username == username && user.password.check(std::move(password));
    });
}

auto Auth::register_user(std::string username, std::string password) -> bool
{
    if (std::ranges::any_of(users_, [&](const auto &user) { return user.username == username; }))
        return false;

    if (std::any_of(username.begin(), username.end(), [](char c) { return std::isalnum(c) == 0; }))
        return false;

    users_.emplace_back(User{std::move(username), Password{std::move(password)}});
    return true;
}
