#pragma once
#include "common.h"
#include <windows.h>
#include <memory>
#include <vector>
#include "thread.h"
#include "count_down_latch.h"
#include "defer.h"

HelperLibBegin

#define SPECIAL_CODE	(-1)

class IocpServer
  :protected NoCopiable
{
  using _Task = std::function<void()>;
public:

  bool Init(WORD asyn_task_count = 0, WORD event_handler_count = 1);

  void Quit();

  void QuitA(DWORD milli_second);

  template<typename Fn, typename ...Args>
  bool PostTask(Fn&& fn, Args&& ...args)
  {
    if (completion_port != INVALID_HANDLE_VALUE) {
      auto t = new _Task{ std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...) };
      if ((DWORD)t != SPECIAL_CODE && t != nullptr) {
        return !!::PostQueuedCompletionStatus(completion_port, SPECIAL_CODE, (ULONG_PTR)t, (LPOVERLAPPED)SPECIAL_CODE);
      }
    }
    return false;
  }

  bool AssociateHandle(HANDLE handle, ULONG_PTR context);
protected:
  void PostExitSignal();

  virtual void OnError(DWORD error_code_win32, ULONG_PTR context, LPOVERLAPPED ol) {}

  virtual void OnComplete(DWORD bytes_transfer, ULONG_PTR context, LPOVERLAPPED ol) {}

  virtual WORD DefaultAsynTaskCount();

private:
  void ThreadFunc();

private:
  HANDLE completion_port{ INVALID_HANDLE_VALUE };

  std::unique_ptr<CountDownLatch> latch_{};

  std::vector<thread> event_handlers_{};
};



HelperLibEnd