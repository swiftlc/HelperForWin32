#pragma once
#include "common.h"
#include "count_down_latch.h"
#include "thread.h"
#include "block_task_queue.h"

HelperLibBegin

template<bool discard_when_quit = false>
class ThreadPool
  :public NoCopiable
{
public:
  ThreadPool(unsigned int thread_count = thread::hardware_concurrency() * 2 + 2)
    :interrupted_state_([=] {block_task_queue_.Notify(true); })
    , latch(thread_count)
  {
    if (!thread_count)return;
    for (unsigned int i = 0; i < thread_count; i++) {
      thread_group_.push_back(thread{ [this]
        {
          while (true) {
            auto t = block_task_queue_.FetchTask(InterruptedFunc(interrupted_state_));
            if (t) {
              (*t)();
            }
            if (interrupted_state_ && (discard_when_quit || !block_task_queue_.GetTaskCount()))break;
            while (interrupted_state_ && block_task_queue_.GetTaskCount() > 0) {
              auto ret = block_task_queue_.FetchTaskA(50ms);
              if (ret.second == WaitReturn::TimeOut)break;
              if (ret.first) {
                (*ret.first)();
              }
            }
          }
          latch.CountDown();
        } });
    }
  }

  template<typename Fn, typename ...Args>
  void AddTask(Fn&& fn, Args&& ...args)
  {
    if (!interrupted_state_) {
      block_task_queue_.AddTask(
        Task::MakeTaskPtr(std::forward<Fn>(fn), std::forward<Args>(args)...));
    }
  }

  void AddTask(std::shared_ptr<Task> task)
  {
    if (!interrupted_state_) {
      block_task_queue_.AddTask(task);
    }
  }

  //@ asyn
  void Quit()
  {
    interrupted_state_ = true;
  }

  void Wait()
  {
    std::for_each(thread_group_.begin(), thread_group_.end(),
      std::bind(&thread::join, std::placeholders::_1));
  }

  template<class Rep, class Period>
  bool WaitA(std::chrono::duration<Rep, Period> rel_time)
  {
    WaitReturn ret = latch.AWait(rel_time);
    if (ret == WaitReturn::Success) {
      Wait();
      return true;
    }
    return false;
  }

  void QuitAndWait()
  {
    Quit();
    Wait();
  }

  template<class Rep, class Period>
  bool QuitAndWaitA(std::chrono::duration<Rep, Period> rel_time)
  {
    Quit();
    return WaitA(rel_time);
  }
private:
  CountDownLatch latch;
  BlockTaskQueue block_task_queue_{};
  InterruptedState interrupted_state_;
  std::vector<thread> thread_group_{};
};

HelperLibEnd