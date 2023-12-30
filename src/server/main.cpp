#include <macrosafe/encrypted_channel.hpp>
#include <shared/message_types.hpp>

using Channel = macrosafe::EncryptedChannel;

// in memory for now
struct Files
{
    std::map<std::filesystem::path, std::vector<std::byte>> files;
};

static inline Files files;

[[nodiscard]] auto handle_upload_request(Channel &channel, const UploadRequest &request) -> bool
{
    try
    {
        files.files[request.path] = request.data;
        auto response             = Response{.data = UploadResponse{.success = true}};
        return channel.send_message(serialize(response)) == macrosafe::SendResult::Success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return false;
    }
}

[[nodiscard]] auto handle_download_request(Channel &channel, const DownloadRequest &request) -> bool
{
    try
    {
        auto response = Response{.data = DownloadResponse{.data = files.files.at(request.path)}};
        return channel.send_message(serialize(response)) == macrosafe::SendResult::Success;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return false;
    }
}

[[nodiscard]] auto handle_list_request(Channel &channel, ListRequest) -> bool
{
    auto ret = std::vector<std::filesystem::path>{};
    for (const auto &[path, _] : files.files)
        ret.emplace_back(path);

    auto response = Response{.data = ListResponse{.files = ret}};
    return channel.send_message(serialize(response)) == macrosafe::SendResult::Success;
}

[[nodiscard]] auto handle_message(Channel &channel, Request request) -> bool
{
    return std::visit(macrosafe::Overload{
                          [&](const UploadRequest &upload_request) {
                              return handle_upload_request(channel, upload_request);
                          },
                          [&](const DownloadRequest &download_request) {
                              return handle_download_request(channel, download_request);
                          },
                          [&](const ListRequest &list_request) {
                              return handle_list_request(channel, list_request);
                          },
                      },
                      request.data);
}

[[nodiscard]] auto receive_message(Channel &channel) -> bool
{
    const auto message = channel.receive_message_blocking();
    if (!message)
        return false;

    const auto parsed_message = deserialize<Request>(message.value());
    if (!parsed_message)
        return false;

    return handle_message(channel, parsed_message.value());
}
auto main() -> int
{
    try
    {
        auto channel = Channel{Channel::ServerConfig{
            .server_port = macrosafe::k_default_server_port,
            .client_port = macrosafe::k_default_client_port,
        }};

        while (true)
        {
            std::cout << "Waiting for message\n" << std::flush;
            auto success = receive_message(channel);
            if (!success)
                std::cerr << "Error receiving message\n";

            std::cout << "Handled message\n" << std::flush;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 2;
    }
}
