#pragma once

#include <thread>

namespace macrosafe {
class JThread
{
    std::thread thread_;

public:
    template<typename Callable, typename... Args>
    explicit JThread(Callable &&f, Args &&...args)
        : thread_(std::forward<Callable>(f), std::forward<Args>(args)...)
    {
    }

    ~JThread()
    {
        if (thread_.joinable())
            thread_.join();
    }

    JThread(const JThread &)                     = delete;
    auto operator=(const JThread &) -> JThread & = delete;

    JThread(JThread &&)                     = default;
    auto operator=(JThread &&) -> JThread & = default;
};
} // namespace macrosafe