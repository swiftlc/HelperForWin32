#pragma once
#include "common.h"
#include <windows.h>
#include <functional>
#include <stdexcept>
#include <process.h>

HelperLibBegin

class thread
{
public:
  using id = unsigned;

  thread() noexcept = default;
  thread(thread&& other) noexcept;
  template<class Function, class... Args >
  explicit thread(Function&& f, Args&&... args)
  {
    std::function<void()>* thread_fn_ = new std::function<void()>{ std::bind(std::forward<Function>(f), std::forward<Args>(args)...) };
    native_handle_ = reinterpret_cast<HANDLE>(_beginthreadex(0, 0, ThreadProc, thread_fn_, 0, &thread_id_));
    if (!joinable()) {
      delete thread_fn_;
      throw std::runtime_error("create thread failed");
    }
  }

  thread(thread const&) = delete;
  thread& operator=(thread const&) = delete;
  thread& operator=(thread&& other) noexcept;

  ~thread();

  id get_id()const noexcept;

  volatile HANDLE* native_handle();

  bool joinable() const noexcept;

  void join();

  //differentt from std
  bool join_for(DWORD mill_second);

  void detach();

  static unsigned int hardware_concurrency() noexcept;
private:
  static unsigned __stdcall ThreadProc(void *param);
  volatile HANDLE native_handle_{ nullptr };
  unsigned thread_id_{ 0 };

  static unsigned int hardware_concurrency_;
};

HelperLibEnd