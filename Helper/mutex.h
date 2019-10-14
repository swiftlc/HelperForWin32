#pragma once
#include "common.h"
#include <windows.h>
#include "unique_lock.h"

HelperLibBegin

class mutex
{
public:
  mutex() noexcept
  {
    ::InitializeCriticalSection(&cs_);
  }

  mutex(mutex const&) = delete;
  mutex& operator=(mutex const&) = delete;

  ~mutex()
  {
    ::DeleteCriticalSection(&cs_);
  }

  void lock()
  {
    ::EnterCriticalSection(&cs_);
  }

  bool try_lock()
  {
    return ::TryEnterCriticalSection(&cs_);
  }

  void unlock()
  {
    ::LeaveCriticalSection(&cs_);
  }

  PCRITICAL_SECTION native_handle()
  {
    return &cs_;
  }

private:
  CRITICAL_SECTION cs_{};
};

struct once_flag
{
  once_flag() noexcept = default;
  once_flag(const once_flag&) = delete;
  once_flag& operator=(const once_flag&) = delete;

  mutex mtx_{};
  volatile bool flag_{ false };
};

template<typename Callable, typename ...Args>
void call_once(once_flag &flag, Callable &&callable, Args&&...asgs)
{
  unique_lock<mutex> Lock{ flag.mtx_ };
  if (!flag.flag_) {
    std::forward<Callable>(callable)(std::forward<Args>(asgs)...);
    flag.flag_ = true;
  }
}

HelperLibEnd