#pragma once
#include "count_down_latch.h"

HelperLibBegin

class WorkSimultaneously
  :protected NoCopiable
{
public:
  WorkSimultaneously(unsigned long worker_count)
    :count_down_latch_(worker_count)
  {
  }

  void Ready()
  {
    count_down_latch_.CountDown();
  }

  //用于中断条件成立时,唤醒沉睡线程进行校验
  void Notify(bool notify_all = true)
  {
    count_down_latch_.Notify(notify_all);
  }

  bool WaitAllReady(PredicateFunc stop_wait_fn = nullptr)
  {
    return count_down_latch_.Wait(stop_wait_fn);
  }

  template<class Rep, class Period>
  WaitReturn AWaitAllReady(std::chrono::duration<Rep, Period> rel_time, PredicateFunc stop_wait_fn = nullptr)
  {
    return count_down_latch_.AWait(rel_time, stop_wait_fn);
  }

  bool ReadyThenWaitAllReady(PredicateFunc stop_wait_fn = nullptr)
  {
    Ready();
    return WaitAllReady(stop_wait_fn);
  }

  template<class Rep, class Period>
  WaitReturn ReadyThenAWaitAllReady(std::chrono::duration<Rep, Period> rel_time, PredicateFunc stop_wait_fn = nullptr)
  {
    Ready();
    return AWaitAllReady(rel_time, stop_wait_fn);
  }

private:
  CountDownLatch count_down_latch_;
};

HelperLibEnd



//WorkSimultaneously latch{ 2 };
//InterruptedState interrupted_state{ [&] {latch.Notify(); } };
//thread{ [&] {
//  Sleep(1000);
//  MyLog << "wait_ready1";
//  latch.ReadyThenAWaitAllReady(3000ms);
//  MyLog << "thread_over1";
//} }.detach();
//
//thread{ [&] {
//Sleep(2000);
//MyLog << "wait_ready2";
//latch.ReadyThenAWaitAllReady(3000ms);
//MyLog << "thread_over2";
//} }.detach();
//
//latch.WaitAllReady();
//MyLog << "main_thread_1";