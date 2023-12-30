#include <macrosafe/channel.hpp>
#include <macrosafe/encrypted_channel.hpp>
#include <macrosafe/utils/metaprogramming.hpp>
#include <shared/message_types.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <variant>
#include <vector>

using Channel = macrosafe::EncryptedChannel;

struct ParsedArgs
{
    struct Transfer
    {
        enum class Mode
        {
            Download,
            Upload,
        } type;

        std::filesystem::path path;
    };

    struct List
    {
    };

    std::variant<Transfer, List> mode;
};

/*
 * Usage:
 * sectrans -up file
 * sectrans -down file
 * sectrans -list
 */
[[nodiscard]] auto parse_args(int argc, char **argv) -> std::optional<ParsedArgs>
{
    assert(argc >= 2);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto args = std::vector<std::string_view>(argv + 1, argv + argc);

    const auto mode = args[0];

    if (mode == "-up")
    {
        if (argc != 3)
            return std::nullopt;

        return ParsedArgs{ParsedArgs::Transfer{ParsedArgs::Transfer::Mode::Upload, args[1]}};
    }

    if (mode == "-down")
    {
        if (argc != 3)
            return std::nullopt;

        return ParsedArgs{ParsedArgs::Transfer{ParsedArgs::Transfer::Mode::Download, args[1]}};
    }

    if (mode == "-list")
    {
        if (argc != 2)
            return std::nullopt;

        return ParsedArgs{ParsedArgs::List{}};
    }

    return std::nullopt;
}

[[nodiscard]] auto validate_args(const ParsedArgs &args) -> bool
{
    auto valid_transfer = [](const ParsedArgs::Transfer &transfer) {
        if (transfer.path.empty())
            return false;

        switch (transfer.type)
        {
            case ParsedArgs::Transfer::Mode::Download:
            {
                auto file           = std::ofstream{transfer.path, std::ios::binary | std::ios::app};
                const bool writable = static_cast<bool>(file);
                return writable;
            }
            case ParsedArgs::Transfer::Mode::Upload:
            {
                auto file           = std::ifstream{transfer.path, std::ios::binary};
                const bool readable = static_cast<bool>(file);
                return readable;
            }
            default: return false;
        }
    };

    return std::visit(macrosafe::Overload{valid_transfer, [](ParsedArgs::List) { return true; }}, args.mode);
}

[[nodiscard]] auto run_upload(const std::filesystem::path &path, Channel &channel) -> bool
{
    auto file = std::ifstream(path, std::ios::binary | std::ios::ate);
    if (!file)
        return false;

    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    auto content = std::vector<std::byte>(static_cast<size_t>(size));
    file.read(reinterpret_cast<char *>(content.data()), size);

    if (file.gcount() != size)
        return false;

    auto message = Request{.data = UploadRequest{.path = path, .data = content}};
    if (channel.send_message(serialize(message)) != macrosafe::SendResult::Success)
        return false;

    auto response = channel.receive_message_blocking();
    if (!response)
        return false;

    auto parsed_response = deserialize<Response>(response.value());
    if (!parsed_response)
        return false;

    return std::visit(macrosafe::Overload{[](const UploadResponse &up_response) {
                                              return up_response.success;
                                          },
                                          [](const auto &) { return false; }},
                      parsed_response.value().data);
}

[[nodiscard]] auto run_download(const std::filesystem::path &path, Channel &channel) -> bool
{
    const auto message = Request{.data = DownloadRequest{.path = path}};
    if (channel.send_message(serialize(message)) != macrosafe::SendResult::Success)
        return false;

    auto response = channel.receive_message_blocking();
    if (!response)
        return false;

    auto parsed_response = deserialize<Response>(response.value());
    if (!parsed_response)
        return false;

    const auto &download_response = std::get<DownloadResponse>(parsed_response.value().data);
    if (!download_response.data)
        return false;

    auto file = std::ofstream(path, std::ios::binary | std::ios::trunc);
    if (!file)
        return false;

    file.write(reinterpret_cast<const char *>(download_response.data->data()),
               static_cast<std::streamsize>(download_response.data->size()));

    return static_cast<bool>(file);
}

[[nodiscard]] auto run_transfer(const ParsedArgs::Transfer &transfer, Channel &channel) -> bool
{
    switch (transfer.type)
    {
        case ParsedArgs::Transfer::Mode::Download: return run_download(transfer.path, channel);
        case ParsedArgs::Transfer::Mode::Upload: return run_upload(transfer.path, channel);
        default: throw std::runtime_error("Invalid transfer mode");
    }
}

[[nodiscard]] auto run_list(ParsedArgs::List, Channel &channel) -> bool
{
    auto message = Request{.data = ListRequest{}};
    if (channel.send_message(serialize(message)) != macrosafe::SendResult::Success)
        return false;

    auto response = channel.receive_message_blocking();
    if (!response)
        return false;

    auto parsed_response = deserialize<Response>(response.value());
    if (!parsed_response)
        return false;

    const auto &list_response = std::get<ListResponse>(parsed_response.value().data);
    if (!list_response.files)
        return false;

    for (const auto &file : list_response.files.value())
        std::cout << file << '\n';

    return true;
}

[[nodiscard]] auto run(const ParsedArgs &args) -> bool
{
    auto channel = Channel{Channel::ClientConfig{
        .server_port = macrosafe::k_default_client_port,
        .client_port = macrosafe::k_default_server_port,
    }};

    return std::visit(macrosafe::Overload{[&channel](const ParsedArgs::Transfer &transfer) {
                                              return run_transfer(transfer, channel);
                                          },
                                          [&channel](const ParsedArgs::List &list) {
                                              return run_list(list, channel);
                                          }},
                      args.mode);
}

auto main(int argc, char **argv) -> int
{
    try
    {
        const auto args_opt = parse_args(argc, argv);
        if (!args_opt)
        {
            std::cerr << "Invalid arguments\n";
            return 1;
        }

        const auto &args = args_opt.value();

        if (!validate_args(args))
        {
            std::cerr << "Valid arguments, but invalid values\n";
            return 2;
        }

        if (!run(args))
        {
            std::cerr << "Error running command\n";
            return 3;
        }

        std::cout << "Success\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 4;
    }

    return 0;
}
