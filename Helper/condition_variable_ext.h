#pragma once
#include "common.h"
#include "condition_variable.h"
#include <chrono>
#include <functional>
#include <memory>

HelperLibBegin

enum class WaitReturn
{
  Success,  //predicate 返回true
  TimeOut,  //等待超时
  Interrupted //中断条件触发
};

inline WaitReturn MakeWaitReturn(bool is_timeout, bool is_interrupted)
{
  return is_timeout ? WaitReturn::TimeOut : (is_interrupted ? WaitReturn::Interrupted : WaitReturn::Success);
}

using PredicateFunc = std::function<bool()>;

class InterruptedState
  :protected NoCopiable
{
public:
  InterruptedState() = default;
  InterruptedState(std::function<void()> notify)
    :notify_(notify)
  {
  }

  InterruptedState& operator=(bool state)
  {
    is_Interrupted = state;
    if (is_Interrupted && notify_) {
      notify_();
    }
    return *this;
  }

  void SetNotify(std::function<void()> notify)
  {
    notify_ = notify;
  }

  operator bool() const
  {
    return is_Interrupted;
  }
protected:
  std::function<void()> notify_{};
  bool volatile is_Interrupted{ false };
};

inline decltype(auto) InterruptedFunc(InterruptedState const&interrupted_state)
{
  return [&] { return !!interrupted_state; };
}

class ConditionVariableExt
  :protected NoCopiable
{
  class PredicateAndInterrupte
  {
  public:
    PredicateAndInterrupte(PredicateFunc predicate, PredicateFunc interrupte)
      :predicate_(predicate), interrupte_(interrupte)
    {
    }

    bool operator()()
    {
      return (interrupte_ && (*is_interrupted_ = interrupte_())) || predicate_();
    }

    bool IsInterrupted()
    {
      return *is_interrupted_;
    }

  private:
    std::shared_ptr<bool> is_interrupted_{ new bool{false} };
    PredicateFunc predicate_{};
    PredicateFunc interrupte_{};
  };

public:
  void Notify(bool notify_all = true)
  {
    notify_all ? cv_.notify_all() : cv_.notify_one();
  }

  bool Wait(unique_lock<mutex>& lock, PredicateFunc predicate, PredicateFunc interrupte = nullptr)
  {
    PredicateAndInterrupte pai{ predicate,interrupte };
    cv_.wait(lock, pai);
    return pai.IsInterrupted();
  }

  template<class Rep, class Period>
  WaitReturn WaitFor(
    unique_lock<mutex>& lock,
    const std::chrono::duration<Rep, Period>& rel_time,
    PredicateFunc predicate,
    PredicateFunc interrupte = nullptr)
  {
    PredicateAndInterrupte pai{ predicate,interrupte };
    bool is_timeout = !cv_.wait_for(lock, rel_time, pai);
    return MakeWaitReturn(is_timeout, pai.IsInterrupted());
  }

  template<class Clock, class Duration>
  WaitReturn WaitUntil(
    unique_lock<mutex>& lock,
    const std::chrono::time_point<Clock, Duration>& timeout_time,
    PredicateFunc predicate,
    PredicateFunc interrupte = nullptr)
  {
    PredicateAndInterrupte pai{ predicate,interrupte };
    bool is_timeout = !cv_.wait_until(lock, timeout_time, pai);
    return MakeWaitReturn(is_timeout, pai.IsInterrupted());
  }

protected:
  condition_variable cv_{};
};

HelperLibEnd