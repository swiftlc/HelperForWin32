#pragma once
#include "common.h"
#include "mutex.h"
#include "unique_lock.h"
#include <functional>

HelperLibBegin

class QuickStartCancel
  :protected NoCopiable
{
  using Lock = helper_lib::unique_lock<helper_lib::mutex>;
public:
  using QuickAction = std::function<void()>;

  QuickStartCancel() = default;
  QuickStartCancel(QuickAction start_action, QuickAction cancel_action)
    :start_action_(start_action), cancel_action_(cancel_action)
  {
  }

  void SetStartAction(QuickAction start_action)
  {
    Lock lk{ mtx_ };
    if (start_action) {
      start_action_ = start_action;
    }
  }

  void SetCancelAction(QuickAction cancel_action)
  {
    Lock lk{ mtx_ };
    if (cancel_action) {
      cancel_action_ = cancel_action;
    }
  }

  void PushStart(QuickAction start_action = nullptr)
  {
    Lock lk{ mtx_ };
    if (start_action)start_action_ = start_action;
    if (is_running_ || is_canceling_) {
      should_start_when_finished_ = true;
      return;
    }
    should_start_when_finished_ = false;
    is_running_ = true;
    start_action_();
  }

  void PushCancel(QuickAction cancel_action)
  {
    Lock lk{ mtx_ };
    if (cancel_action)cancel_action_ = cancel_action;
    if (is_canceling_) {
      should_start_when_finished_ = false;
      return;
    }
    if (is_running_) {
      is_running_ = false;
      is_canceling_ = true;
      cancel_action_();
      return;
    }
  }

  //invoke when task end
  void OnTaskEnd()
  {
    Lock lk{ mtx_ };
    is_running_ = false;
    is_canceling_ = false;

    if (should_start_when_finished_) {
      should_start_when_finished_ = false;
      is_running_ = true;
      start_action_();
    }
  }

private:
  QuickAction start_action_;
  QuickAction cancel_action_;

  helper_lib::mutex mtx_{};

  bool is_running_{ false };
  bool is_canceling_{ false };
  bool should_start_when_finished_{ false };
};

HelperLibEnd




//class Notify
//{
//public:
//  virtual void OnTask() = 0;
//  virtual void OnEnd(bool is_success) = 0;
//};
//
//class RunTask
//{
//public:
//  void Run(Notify *notify)
//  {
//    if (is_running_)return;
//    notify_ = notify;
//    is_running_ = true;
//    thread{ [this] {
//      DEFER([this] {is_running_ = false; is_stop_ = false; });
//      for (int i = 0; i < 5; i++) {
//        Sleep(1000);
//        notify_->OnTask();
//        if (is_stop_) {
//          Sleep(3000);
//          notify_->OnEnd(false);
//          return;
//        }
//    }
//      notify_->OnEnd(true);
//    } }.detach();
//
//  }
//  void Cancel()
//  {
//    if (is_stop_)return;
//    is_stop_ = true;
//  }
//private:
//  Notify *notify_;
//
//  volatile bool is_running_{};
//  volatile bool is_stop_{};
//};

