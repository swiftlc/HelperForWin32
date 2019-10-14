#pragma once
#include "common.h"
#include "task.h"
#include "condition_variable_ext.h"
#include <queue>
#include <memory>
#include <vector>

HelperLibBegin

class BlockTaskQueue
  :protected NoCopiable
{
  using TaskPtr = std::shared_ptr<Task>;
  using Lock = unique_lock<mutex>;
public:
  BlockTaskQueue(unsigned capacity = 0 /*0 no limit*/)
    :capacity_(capacity)
  {
  }

  //用于中断条件成立时,唤醒沉睡线程进行校验
  void Notify(bool is_condition_queue_not_empty, bool notify_all = true)
  {
    if (is_condition_queue_not_empty) {
      task_queue_is_not_empty_.Notify(notify_all);
    } else {
      task_queue_is_not_full_.Notify(notify_all);
    }
  }

  bool AddTask(TaskPtr task, PredicateFunc interrupte = nullptr)
  {
    Lock lk{ mtx_ };
    if (capacity_ > 0) {
      if (task_queue_is_not_full_.Wait(lk, [this] {return task_queue_.size() < capacity_; }, interrupte)) {
        return true;
      }
    }
    task_queue_.push(task);
    task_queue_is_not_empty_.Notify();
    return false;
  }

  template<class Rep, class Period>
  WaitReturn AddTaskA(TaskPtr task, std::chrono::duration<Rep, Period> rel_time, PredicateFunc interrupte = nullptr)
  {
    Lock lk{ mtx_ };
    WaitReturn ret{ WaitReturn::Success };
    if (capacity_ > 0) {
      ret = task_queue_is_not_full_.WaitFor(lk, rel_time, [this] {return task_queue_.size() < capacity_; }, interrupte);
    }
    if (ret == WaitReturn::Success) {
      task_queue_.push(task);
      task_queue_is_not_empty_.Notify();
    }
    return ret;
  }

  TaskPtr FetchTask(PredicateFunc interrupte = nullptr)
  {
    Lock lk{ mtx_ };
    if (task_queue_is_not_empty_.Wait(lk, [this] {return task_queue_.size() > 0; }, interrupte)) {
      return nullptr;
    }
    auto ret = task_queue_.front();
    task_queue_.pop();
    task_queue_is_not_full_.Notify();
    return ret;
  }

  template<class Rep, class Period>
  std::pair<TaskPtr, WaitReturn> FetchTaskA(std::chrono::duration<Rep, Period> rel_time, PredicateFunc interrupte = nullptr)
  {
    Lock lk{ mtx_ };
    WaitReturn wait_return = task_queue_is_not_empty_.WaitFor(lk, rel_time, [this] {return task_queue_.size() > 0; }, interrupte);
    if (wait_return == WaitReturn::Success) {
      auto task = task_queue_.front();
      task_queue_.pop();
      task_queue_is_not_full_.Notify();
      return { task,wait_return };
    }
    return { nullptr,wait_return };
  }

  decltype(auto) GetTaskCount()
  {
    Lock lk{ mtx_ };
    return task_queue_.size();
  }

  std::queue<TaskPtr> FetchAllTask()
  {
    Lock lk{ mtx_ };
    return std::move(task_queue_);
  }
protected:
  std::queue<TaskPtr> task_queue_{};
  unsigned capacity_{};
  mutex mtx_{};
  ConditionVariableExt task_queue_is_not_full_{};
  ConditionVariableExt task_queue_is_not_empty_{};
};

HelperLibEnd


//BlockTaskQueue block_task_queue_{};
//InterruptedState interrupted_state{ [&block_task_queue_] {block_task_queue_.Notify(true); } };
//for (int i = 0; i < 10; i++) {
//  thread{ [i,&block_task_queue_,&interrupted_state]
//  {
//    while (true) {
//      auto task = block_task_queue_.FetchTask(InterruptedFunc(interrupted_state));
//      if (task) {
//        (*task)();
//      } else {
//        MyLog << "thread" << i << "end";
//        break;
//      }
//}
//    }
//  }.detach();
//}
//
//for (int i = 0; i < 10; i++) {
//  block_task_queue_.AddTask(Task::MakeTask([i]
//  {
//    MyLog << "Task" << i;
//  }));
//  Sleep(500);
//}
//auto serial_task1 = SerialTask::MakeSerialTask(nullptr, [] {MyLog << "task1"; });
//auto serial_task2 = SerialTask::MakeSerialTask(serial_task1, [] {MyLog << "task2"; });
//auto serial_task3 = SerialTask::MakeSerialTask(serial_task2, [] {MyLog << "task3"; });
//
//block_task_queue_.AddTask(serial_task3);
//Sleep(1000);
//block_task_queue_.AddTask(serial_task2);
//Sleep(1000);
//block_task_queue_.AddTask(serial_task1);
//Sleep(1000);
//
//interrupted_state = true;