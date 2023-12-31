#include "hydrogen.h"

#include <algorithm>
#include <array>
#include <stdexcept>
#include <vector>

namespace macrosafe::detail {

template<size_t N>
[[nodiscard]] auto from_u8(const std::array<uint8_t, N> &arr) -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{N};
    std::transform(arr.begin(), arr.end(), result.begin(), [](auto elem) { return std::byte{elem}; });
    return result;
}

inline void require_packet_size(const std::vector<std::byte> &data, size_t size) noexcept(false)
{
    if (data.size() != size)
        throw std::runtime_error("Invalid packet size. Expected " + std::to_string(size) + ", got "
                                 + std::to_string(data.size()));
}

class CryptoSession
{
    hydro_kx_session_keypair session_keypair_{};

public:
    explicit CryptoSession(hydro_kx_session_keypair session_keypair)
        : session_keypair_(session_keypair)
    {
    }

    CryptoSession() = default;

    [[nodiscard]] auto encrypt(std::span<const std::byte> data) -> std::vector<std::byte>
    {
        auto result = std::vector<uint8_t>{};
        result.resize(data.size() + hydro_secretbox_HEADERBYTES);
        if (hydro_secretbox_encrypt(result.data(),
                                    data.data(),
                                    data.size(),
                                    0,
                                    "macrosafe",
                                    static_cast<uint8_t *>(session_keypair_.tx))
            != 0)
        {
            throw std::runtime_error("hydro_secretbox_encrypt() failed");
        }

        return {reinterpret_cast<std::byte *>(result.data()),
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                reinterpret_cast<std::byte *>(result.data() + result.size())};
    }

    [[nodiscard]] auto decrypt(std::span<const std::byte> data) -> std::vector<std::byte>
    {
        auto result = std::vector<uint8_t>{};
        result.resize(data.size() - hydro_secretbox_HEADERBYTES);
        if (hydro_secretbox_decrypt(result.data(),
                                    reinterpret_cast<const uint8_t *>(data.data()),
                                    data.size(),
                                    0,
                                    "macrosafe",
                                    static_cast<uint8_t *>(session_keypair_.rx))
            != 0)
        {
            throw std::runtime_error("hydro_secretbox_decrypt() failed");
        }

        return {reinterpret_cast<std::byte *>(result.data()),
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                reinterpret_cast<std::byte *>(result.data() + result.size())};
    }
};

class CryptoSessionHelperBase
{
protected:
    hydro_kx_keypair own_keypair_{};
    hydro_kx_state state_{};

    CryptoSessionHelperBase()
    {
        if (hydro_init() != 0)
            throw std::runtime_error("hydro_init() failed");

        hydro_kx_keygen(&own_keypair_);
    }
};

class CryptoSessionHelperClient : private CryptoSessionHelperBase
{
public:
    [[nodiscard]] auto initial_packet()
    {
        std::array<uint8_t, hydro_kx_XX_PACKET1BYTES> packet1{};
        hydro_kx_xx_1(&state_, packet1.data(), nullptr);
        return from_u8(packet1);
    }

    [[nodiscard]] auto final_packet(const std::vector<std::byte> &packet2)
    {
        require_packet_size(packet2, hydro_kx_XX_PACKET2BYTES);

        hydro_kx_session_keypair session_keypair{};
        std::array<uint8_t, hydro_kx_XX_PACKET3BYTES> packet3{};

        if (hydro_kx_xx_3(&state_,
                          &session_keypair,
                          packet3.data(),
                          nullptr,
                          reinterpret_cast<const uint8_t *>(packet2.data()),
                          nullptr,
                          &own_keypair_)
            != 0)
        {
            throw std::runtime_error("hydro_kx_xx_3() failed");
        }

        struct
        {
            std::vector<std::byte> packet;
            CryptoSession session;
        } ret{from_u8(packet3), CryptoSession{session_keypair}};
        return ret;
    }
};

class CryptoSessionHelperServer : private CryptoSessionHelperBase
{
public:
    [[nodiscard]] auto process_initial(const std::vector<std::byte> &packet1)
    {
        require_packet_size(packet1, hydro_kx_XX_PACKET1BYTES);

        std::array<uint8_t, hydro_kx_XX_PACKET2BYTES> packet2{};
        if (hydro_kx_xx_2(&state_,
                          packet2.data(),
                          reinterpret_cast<const uint8_t *>(packet1.data()),
                          nullptr,
                          &own_keypair_)
            != 0)
        {
            throw std::runtime_error("hydro_kx_xx_2() failed");
        }

        return from_u8(packet2);
    }

    [[nodiscard]] auto process_final(const std::vector<std::byte> &packet3)
    {
        require_packet_size(packet3, hydro_kx_XX_PACKET3BYTES);
        hydro_kx_session_keypair session_keypair{};

        if (hydro_kx_xx_4(&state_,
                          &session_keypair,
                          nullptr,
                          reinterpret_cast<const uint8_t *>(packet3.data()),
                          nullptr)
            != 0)
        {
            throw std::runtime_error("hydro_kx_xx_4() failed");
        }

        return CryptoSession{session_keypair};
    }
};

} // namespace macrosafe::detail
