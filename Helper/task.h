#pragma once
#include "common.h"
#include <functional>
#include <memory>

HelperLibBegin

class Task
{
public:
  virtual ~Task() {}
  virtual void operator()()
  {
    task_();
  }

  template<typename Fn, typename ...Args>
  static std::shared_ptr<Task> MakeTaskPtr(Fn &&fn, Args &&...args)
  {
    Task *t = new Task{};
    t->task_ = std::bind(std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
    return std::shared_ptr<Task>(t);
  }

  template<typename Fn, typename ...Args>
  static Task MakeTask(Fn &&fn, Args &&...args)
  {
    Task t;
    t.task_ = std::bind(std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
    return t;
  }

protected:
  std::function<void()> task_;
};

HelperLibEnd