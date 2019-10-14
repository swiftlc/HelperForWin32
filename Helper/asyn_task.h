#pragma once
#include "common.h"
#include <windows.h>
#include <assert.h>
#include <functional>
#include <memory>

HelperLibBegin

#define kAsyncTask (WM_USER + 1)

class AsyncTask
{
protected:
  using Task = std::function<void(void)>;
public:
  AsyncTask(Task&& task) : task_(std::move(task)) {};
  AsyncTask(Task const& task) : task_(task) {};
  void SetReleaseFn(Task const&release_fn) { release_fn_ = release_fn; }
  virtual ~AsyncTask() { if (release_fn_)release_fn_(); }
  void Run() { if (task_) task_(); }
  static inline bool Post(HWND notify, AsyncTask* task)
  {
    bool ok = !!::PostMessage(notify, kAsyncTask, 0, LPARAM(task));
    if (!ok) {
      assert(false);
      delete task;
    }
    return ok;
  }

  static inline bool Post(HWND notify, Task const&task, Task const&release_fn = {})
  {
    auto _task = new AsyncTask{ task };
    _task->SetReleaseFn(release_fn);
    return Post(notify, _task);
  }

private:
  Task task_;
  Task release_fn_;
};

template<class T>
class AsyncTaskT
  : public AsyncTask
{
public:
  template<class _Type>
  AsyncTaskT(Task const&task, _Type&& data)
    : AsyncTask(task)
    , data_(std::forward<_Type>(data))
  {
  }

private:
  std::shared_ptr<T> data_{};
};

inline void ExecuteAsynTask(LPARAM lParam)
{
  AsyncTask* task = reinterpret_cast<AsyncTask*>(lParam);
  task->Run();
  delete task;
}

HelperLibEnd