#pragma once
#ifndef MANIFOLD_FUTURE_HPP
#define MANIFOLD_FUTURE_HPP

#include <future>
#include <experimental/coroutine>

namespace manifold
{
  template<typename T>
  class future
  {
  public:
    class promise_type
    {
    public:
      promise_type() :
        c_(std::make_shared<std::experimental::coroutine_handle<>>())
      { }
      auto get_return_object() { return future<T>(p_.get_future(), c_); }
      std::experimental::suspend_never initial_suspend() { return {}; }
      std::experimental::suspend_never final_suspend() { return {}; }
      void set_exception(std::exception_ptr e) { p_.set_exception(std::move(e)); }
      void unhandled_exception() { std::terminate(); }
      template <typename U>
      void return_value(U &&u)
      {
        p_.set_value(std::forward<U>(u));
        if (*c_)
          (*c_)();
      }
    private:
      std::promise<T> p_;
      std::shared_ptr<std::experimental::coroutine_handle<>> c_;
    };

    future(std::future<T>&& f, const std::shared_ptr<std::experimental::coroutine_handle<>>& c) :
      fut_(std::move(f)),
      coro_(c)
    { }

    bool await_ready() { return fut_.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }
    T await_resume() { return fut_.get(); }
    void await_suspend(std::experimental::coroutine_handle<> coro) { *coro_ = coro; }
  private:
    std::future<T> fut_;
    std::shared_ptr<std::experimental::coroutine_handle<>> coro_;
  };

  template<>
  class future<void>
  {
  public:
    class promise_type
    {
    public:
      promise_type() :
        c_(std::make_shared<std::experimental::coroutine_handle<>>())
      { }
      auto get_return_object() { return future<void>(p_.get_future(), c_); }
      std::experimental::suspend_never initial_suspend() { return {}; }
      std::experimental::suspend_never final_suspend() { return {}; }
      void set_exception(std::exception_ptr e) { p_.set_exception(std::move(e)); }
      void unhandled_exception() { std::terminate(); }
      void return_void()
      {
        p_.set_value();
        if (*c_)
          (*c_)();
      }
    private:
      std::promise<void> p_;
      std::shared_ptr<std::experimental::coroutine_handle<>> c_;
    };

    future(std::future<void>&& f, const std::shared_ptr<std::experimental::coroutine_handle<>>& c) :
      fut_(std::move(f)),
      coro_(c)
    { }

    bool await_ready() { return fut_.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }
    void await_resume() { fut_.get(); }
    void await_suspend(std::experimental::coroutine_handle<> coro) { *coro_ = coro; }
  private:
    std::future<void> fut_;
    std::shared_ptr<std::experimental::coroutine_handle<>> coro_;
  };
}

//template <typename R, typename... Args>
//struct std::experimental::coroutine_traits<std::future<R>, Args...>
//{
//  struct promise_type
//  {
//    std::promise<R> p;
//    auto get_return_object()
//    {
//      return p.get_future();
//    }
//    std::experimental::suspend_never initial_suspend()
//    {
//      return { };
//    }
//    std::experimental::suspend_never final_suspend()
//    {
//      return { };
//    }
//    void unhandled_exception()
//    {
//      std::terminate();
//    }
//    void set_exception(std::exception_ptr e)
//    {
//      p.set_exception(std::move(e));
//    }
//    template <typename U> void return_value(U &&u)
//    {
//      p.set_value(std::forward<U>(u));
//    }
//  };
//};
//template <typename... Args>
//struct std::experimental::coroutine_traits<std::future<void>, Args...>
//{
//  struct promise_type
//  {
//    std::promise<void> p;
//    auto get_return_object()
//    {
//      return p.get_future();
//    }
//    std::experimental::suspend_never initial_suspend()
//    {
//      return { };
//    }
//    std::experimental::suspend_never final_suspend()
//    {
//      return { };
//    }
//
//    void unhandled_exception()
//    {
//      std::terminate();
//    }
//
//    void set_exception(std::exception_ptr e)
//    {
//      p.set_exception(std::move(e));
//    }
//    void return_void()
//    {
//      p.set_value();
//    }
//  };
//};

#endif //MANIFOLD_FUTURE_HPP

