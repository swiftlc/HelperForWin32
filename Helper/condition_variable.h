#pragma once
#include "common.h"
#include <windows.h>
#include <chrono>
#include "unique_lock.h"
#include "mutex.h"
#include "condition_variable_xp.h"

using namespace std::literals::chrono_literals;

HelperLibBegin

class condition_variable
{
public:
  condition_variable()
  {
    InitializeConditionVariableXp(&cv_);
  }

  condition_variable(const condition_variable&) = delete;
  condition_variable& operator=(const condition_variable&) = delete;

  ~condition_variable() = default;

  void notify_one() noexcept
  {
    WakeConditionVariableXp(&cv_);
  }

  void notify_all() noexcept
  {
    WakeAllConditionVariableXp(&cv_);
  }

  void wait(unique_lock<mutex>& lock)
  {
    SleepConditionVariableXp(&cv_, lock.mutex()->native_handle(), INFINITE);
  }

  template<class Predicate>
  void wait(unique_lock<mutex>& lock, Predicate pred)
  {
    while (!pred()) {
      wait(lock);
    }
  }

  //true -> time_out
  template< class Rep, class Period >
  bool wait_for(unique_lock<mutex>& lock,
    const std::chrono::duration<Rep, Period>& rel_time)
  {
    //If the function fails or the time-out interval elapses, the return value is zero
    !SleepConditionVariableXp(&cv_, lock.mutex()->native_handle(), (DWORD)std::chrono::duration_cast<std::chrono::milliseconds>(rel_time).count());
  }

  template< class Rep, class Period, class Predicate >
  bool wait_for(unique_lock<mutex>& lock,
    const std::chrono::duration<Rep, Period>& rel_time,
    Predicate pred)
  {
    using Clock = std::chrono::steady_clock;
    auto end = Clock::now() + rel_time;
    return wait_until(lock, end, pred);
  }

  //true -> time_out
  template< class Clock, class Duration >
  bool wait_until(unique_lock<mutex>& lock,
    const std::chrono::time_point<Clock, Duration>& timeout_time)
  {
    auto now = Clock::now();
    DWORD millisec = (DWORD)std::chrono::duration_cast<std::chrono::milliseconds>().count();
    if (millisec <= 0)return true;
    return !SleepConditionVariableXp(&cv_, lock.mutex()->native_handle(), millisec);
  }

  template< class Clock, class Duration, class Pred >
  bool wait_until(unique_lock<mutex>& lock,
    const std::chrono::time_point<Clock, Duration>& timeout_time,
    Pred pred)
  {
    while (!pred()) {
      auto now = Clock::now();
      if (now >= timeout_time)return false;
      DWORD millisec = (DWORD)std::chrono::duration_cast<std::chrono::milliseconds>(timeout_time - Clock::now()).count();
      if (millisec > 0) {
        SleepConditionVariableXp(&cv_, lock.mutex()->native_handle(), millisec);
      } else {
        return false;
      }
    }
    return true;
  }

  LPCONDITION_VARIABLE_XP native_handle()
  {
    return &cv_;
  }

private:
  CONDITION_VARIABLE_XP cv_{};
};

HelperLibEnd