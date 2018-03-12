#pragma once
#ifndef MANIFOLD_FUTURE_HPP
#define MANIFOLD_FUTURE_HPP

#include <future>
#include <experimental/coroutine>

template <typename R, typename... Args>
struct std::experimental::coroutine_traits<std::future<R>, Args...>
{
  struct promise_type
  {
    std::promise<R> p;
    auto get_return_object()
    {
      return p.get_future();
    }
    std::experimental::suspend_never initial_suspend()
    {
      return { };
    }
    std::experimental::suspend_never final_suspend()
    {
      return { };
    }
    void unhandled_exception()
    {
      std::terminate();
    }
    void set_exception(std::exception_ptr e)
    {
      p.set_exception(std::move(e));
    }
    template <typename U> void return_value(U &&u)
    {
      p.set_value(std::forward<U>(u));
    }
  };
};
template <typename... Args>
struct std::experimental::coroutine_traits<std::future<void>, Args...>
{
  struct promise_type
  {
    std::promise<void> p;
    auto get_return_object()
    {
      return p.get_future();
    }
    std::experimental::suspend_never initial_suspend()
    {
      return { };
    }
    std::experimental::suspend_never final_suspend()
    {
      return { };
    }

    void unhandled_exception()
    {
      std::terminate();
    }

    void set_exception(std::exception_ptr e)
    {
      p.set_exception(std::move(e));
    }
    void return_void()
    {
      p.set_value();
    }
  };
};

#endif //MANIFOLD_FUTURE_HPP

