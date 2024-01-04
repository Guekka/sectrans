#pragma once

#include <shared/json.hpp>

#include <filesystem>
#include <optional>
#include <variant>
#include <vector>

struct UploadRequest
{
    std::filesystem::path path;
    std::vector<std::byte> data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UploadRequest, path, data)

struct DownloadRequest
{
    std::filesystem::path path;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DownloadRequest, path)

struct ListRequest
{
    bool dummy; // helps with serialization. Only temporary.
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ListRequest, dummy)

struct Request
{
    std::variant<UploadRequest, DownloadRequest, ListRequest> data;
    std::string username;
    std::string password;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Request, data, username, password)

struct UploadResponse
{
    bool success;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UploadResponse, success)

struct DownloadResponse
{
    std::optional<std::vector<std::byte>> data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DownloadResponse, data)

struct ListResponse
{
    std::optional<std::vector<std::filesystem::path>> files;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ListResponse, files)

struct GenericError
{
    std::string message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GenericError, message)

struct Response
{
    std::variant<UploadResponse, DownloadResponse, ListResponse, GenericError> data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Response, data)

template<typename T>
[[nodiscard]] auto serialize(const T &data) -> std::vector<std::byte>
{
    auto msgpack = nlohmann::json::to_msgpack(data);
    return {std::launder(reinterpret_cast<const std::byte *>(msgpack.data())),
            std::launder(reinterpret_cast<const std::byte *>(msgpack.data() + msgpack.size()))};
}

template<typename T>
[[nodiscard]] auto deserialize(const std::vector<std::byte> &data) -> std::optional<T>
{
    auto json = nlohmann::json::from_msgpack(
        {std::launder(reinterpret_cast<const char *>(data.data())),
         // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
         std::launder(reinterpret_cast<const char *>(data.data() + data.size()))});
    return json.get<T>();
}
