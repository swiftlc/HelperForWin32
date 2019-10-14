#pragma once
#include "count_down_latch.h"
#include "task.h"

HelperLibBegin

class SerialTask
  :public Task
{
public:
  virtual void operator()()
  {
    if (pre_task_) {
      pre_task_->task_over_.Wait();
    }
    __super::operator()();
    task_over_.CountDown();
  }

  template<typename Fn, typename ...Args>
  static std::shared_ptr<SerialTask> MakeSerialTask(std::shared_ptr<SerialTask> pre_task, Fn &&fn, Args &&...args)
  {
    SerialTask *t = new SerialTask{};
    t->pre_task_ = pre_task;
    t->task_ = std::bind(std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
    return std::shared_ptr<SerialTask>{t};
  }
protected:
  std::shared_ptr<SerialTask> pre_task_{};
  CountDownLatch task_over_{ 1 };
};

HelperLibEnd
