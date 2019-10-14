#pragma once
#include "common.h"
#include "condition_variable_ext.h"

HelperLibBegin

class CountDownLatch
  :protected NoCopiable
{
protected:
  using Lock = unique_lock<mutex>;
public:
  CountDownLatch(unsigned long count)
    :count_(count) {}

  void CountDown()
  {
    //double check
    if (count_ > 0) {
      Lock lk{ mtx_ };
      if (count_ > 0 && --count_ == 0) {
        cv_.Notify();
      }
    }
  }

  //用于中断条件成立时,唤醒沉睡线程进行校验
  void Notify(bool notify_all = true)
  {
    cv_.Notify(notify_all);
  }

  bool Wait(PredicateFunc interrupte = nullptr)
  {
    Lock lk{ mtx_ };
    return cv_.Wait(lk, [=] {return count_ == 0; }, interrupte);
  }

  template<class Rep, class Period>
  WaitReturn AWait(std::chrono::duration<Rep, Period> rel_time, PredicateFunc interrupte = nullptr)
  {
    Lock lk{ mtx_ };
    return cv_.WaitFor(lk, rel_time, [=] {return count_ == 0; }, interrupte);
  }

  //true 为被中断
  bool CountDownAndWait(PredicateFunc interrupte = nullptr)
  {
    CountDown();
    return Wait(interrupte);
  }

  template<class Rep, class Period>
  WaitReturn CountDownAndAWait(std::chrono::duration<Rep, Period> rel_time, PredicateFunc interrupte = nullptr)
  {
    CountDown();
    return AWait(duration, interrupte);
  }

  unsigned long GetCount()
  {
    Lock lk{ mtx_ };
    return count_;
  }

protected:
  mutex mtx_{};
  ConditionVariableExt cv_{};
  unsigned long count_{};
};

class QuitLatch
  :protected CountDownLatch
{
public:
  QuitLatch()
    :CountDownLatch(1)
  {

  }

  void Reset()
  {
    Lock lk{ mtx_ };
    count_ = 1;
  }

  bool IsQuit()
  {
    return !GetCount();
  }

  //可被quit打断sleep
  template<class Rep, class Period>
  WaitReturn InterruptedSleep(std::chrono::duration<Rep, Period> rel_time, PredicateFunc interrupte = nullptr)
  {
    return AWait(rel_time, interrupte);
  }

  void Quit()
  {
    CountDown();
  }
};

HelperLibEnd



//CountDownLatch latch{ 1 };
//InterruptedState interrupted_state{ [&] {latch.Notify(); } };
//thread{ [&] {
//  MyLog << "wait_ready";
//  latch.AWait(3000ms, InterruptedFunc(interrupted_state));
//  MyLog << "thread_over";
//} }.detach();
//
//Sleep(1000);
//MyLog << "interrupt";
//interrupted_state = true;
//Sleep(5000);
//MyLog << "main_thread_1"; 