#pragma once
#include "future.hpp"

namespace saucer
{
    template <typename... T>
    auto all(std::future<T> &...futures)
    {
        return all(std::move(futures)...);
    }

    template <typename... T>
    auto all(std::future<T>... futures)
    {
        return std::tuple_cat(
            []<typename F>(std::future<F> future)
            {
                if constexpr (!std::same_as<F, void>)
                {
                    return std::make_tuple(future.get());
                }
                else
                {
                    return std::tuple<>();
                }
            }(std::move(futures)...));
    }

    template <typename T, typename Callback>
    void then(std::future<T> future, Callback &&callback)
    {
        auto fut = std::make_shared<std::future<void>>();

        *fut = std::async(std::launch::async,
                          [fut, future = std::move(future), callback = std::forward<Callback>(callback)]() mutable
                          { callback(future.get()); });
    }

    template <typename Callback>
    class then_pipe
    {
        Callback m_callback;

      public:
        then_pipe(Callback callback) : m_callback(std::move(callback)) {}

      public:
        template <typename T>
        friend void operator|(std::future<T> &&future, then_pipe pipe)
        {
            then(std::move(future), std::move(pipe.m_callback));
        }
    };

    template <typename Callback>
    then_pipe<Callback> then(Callback &&callback)
    {
        return then_pipe{std::forward<Callback>(callback)};
    }

    template <typename T>
    void forget(std::future<T> future)
    {
        auto fut = std::make_shared<std::future<void>>();
        *fut = std::async(std::launch::async, [fut, future = std::move(future)]() mutable { future.get(); });
    }

    struct forget_pipe
    {
        template <typename T>
        friend void operator|(std::future<T> future, [[maybe_unused]] forget_pipe)
        {
            forget(std::move(future));
        }
    };

    inline forget_pipe forget()
    {
        return forget_pipe{};
    }
} // namespace saucer