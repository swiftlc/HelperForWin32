#pragma once
#include "common.h"
#include <windows.h>
#include <functional>
#include "event_observer.h"
#include "asyn_task.h"
#include "mutex.h"
#include "thread.h"
#include "count_down_latch.h"

HelperLibBegin

class ThreadMessageLoop
  :protected NoCopiable
{
  using Task = std::function<void()>;
public:
  //UINT message, WPARAM wParam, LPARAM lParam, bool &handled
  Events<UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/, bool &/*handled*/> on_message_;

  ThreadMessageLoop()
  {
    t_ = thread{ [=] {
      MSG msg;
      ::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
      if (is_stopped_) return;
      latch_.CountDown();
      while (::GetMessage(&msg, NULL, 0, 0)) {
        bool handled = false;
        on_message_(msg.message, msg.wParam, msg.lParam, handled);
        if (handled)continue;
        if (msg.message == kAsyncTask) {
          auto t = (Task*)msg.lParam;
          if (t) {
            (*t)();
            delete t;
          }
        }
      }
    }
    };
  }

  DWORD GetThreadID() const { return t_.get_id(); }

  bool PostMessage(UINT message, WPARAM wParam, LPARAM lParam)
  {
    if (is_stopped_) return false;
    if (IsReady()) {
      return !!::PostThreadMessage(GetThreadID(), message, wParam, lParam);
    }
    return false;
  }

  bool PostTask(Task const& task)
  {
    if (is_stopped_) return false;
    Task *t = new Task{ task };
    if (!PostMessage(kAsyncTask, 0, (LPARAM)t)) {
      delete t;
      return false;
    }
    return true;
  }

  //同步任务
  bool SendTask(Task const& task)
  {
    if (is_stopped_) return false;
    CountDownLatch latch{ 1 };
    Task *t = new Task{ [&] {task(); latch.CountDown(); } };
    if (!PostMessage(kAsyncTask, 0, (LPARAM)t)) {
      delete t;
      return false;
    }
    latch.Wait();
    return true;
  }

  bool SendTaskTimeout(Task const& task, int mill_second_timeout)
  {
    if (is_stopped_) return false;
    auto latch = std::make_shared<CountDownLatch>(1);
    Task *t = new Task{ [=] {task(); latch->CountDown(); } };
    if (!PostMessage(kAsyncTask, 0, (LPARAM)t)) {
      delete t;
      return false;
    }
    latch->AWait(mill_second_timeout * 1ms);
    return true;
  }

  bool IsStopped() const volatile { return is_stopped_; }

  //@asyn
  void Stop()
  {
    is_stopped_ = true;
    ::PostThreadMessage(GetThreadID(), WM_QUIT, 0, 0);
  }

  void Join()
  {
    if (t_.joinable()) t_.join();
  }

  bool JoinFor(DWORD mill_second)
  {
    if (!t_.joinable())return true;
    return t_.join_for(mill_second);
  }

  bool WaitReady(PredicateFunc interrupte = nullptr)
  {
    return latch_.Wait(interrupte);
  }

  template<class Rep, class Period>
  WaitReturn AWaitReady(std::chrono::duration<Rep, Period> rel_time, PredicateFunc interrupte = nullptr)
  {
    return latch_.AWait(rel_time, interrupte);
  }

  bool IsReady()
  {
    return latch_.GetCount() == 0;
  }

protected:
  thread t_{};
  CountDownLatch latch_{ 1 };

  volatile bool is_stopped_{ false };
};


HelperLibEnd


//{
//  ThreadMessageLoop tml;
//
//  MyLog << tml.IsReady();
//  MyLog << tml.IsReady();
//  MyLog << tml.IsReady();
//
//  if (tml.IsReady()) {
//    tml.PostTask([] {MyLog << 123; });
//    tml.PostTask([] {Sleep(3000); MyLog << 321;  });
//    Sleep(100);
//  }
//  tml.Stop();
//  tml.JoinFor(1000);
//}
//
//MyLog << "over";



//ThreadMessageLoop tml{};
//tml.WaitReady();
//
//BeginPerf();
//tml.SendTaskTimeout([&] {Sleep(5000); EndPerf(a); MyLog << "do task,cast: " << a; }, 3000);
//EndPerf(a);
//MyLog << "after send task,cast:" << a;