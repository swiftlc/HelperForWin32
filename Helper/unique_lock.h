#pragma once
#include "common.h"
#include <windows.h>

HelperLibBegin

template<typename Mutex>
class unique_lock
{
  //...目前只支持mutex ...后期根据需要扩展
public:
  using mutex_type = Mutex;

  unique_lock() noexcept = default;

  explicit unique_lock(mutex_type& m)
    :mutex_ptr_outter_(&m)
  {
    lock();
  }

  unique_lock(unique_lock&& x)
    :mutex_ptr_outter_(x.mutex_ptr_outter_),
    owns_lock_(x.owns_lock_)
  {
    x.mutex_ptr_outter_ = nullptr;
    x.owns_lock_ = false;
  }

  unique_lock(const unique_lock&) = delete;
  unique_lock& operator= (const unique_lock&) = delete;

  unique_lock& operator= (unique_lock&& x) noexcept
  {
    unlock();

    mutex_ptr_outter_ = x.mutex_ptr_outter_;
    owns_lock_ = x.owns_lock_;

    x.mutex_ptr_outter_ = nullptr;
    x.owns_lock_ = false;
    return *this;
  }

  ~unique_lock()
  {
    unlock();
  }

  void lock()
  {
    if (mutex_ptr_outter_ && !owns_lock_) {
      mutex_ptr_outter_->lock();
      owns_lock_ = true;
    }
  }

  bool try_lock()
  {
    if (mutex_ptr_outter_ && !owns_lock_) {
      if (mutex_ptr_outter_->try_lock()) {
        owns_lock_ = true;
      }
    }
    return owns_lock_;
  }

  void unlock()
  {
    if (mutex_ptr_outter_ && owns_lock_) {
      mutex_ptr_outter_->unlock();
      owns_lock_ = false;
    }
  }

  bool owns_lock() const noexcept
  {
    return owns_lock_;
  }

  explicit operator bool() const noexcept
  {
    return owns_lock_;
  }

  //放弃持有mutex锁,如果先前持有锁，则释放锁
  mutex_type* release() noexcept
  {
    unlock();

    mutex_type* ret = mutex_ptr_outter_;

    mutex_ptr_outter_ = nullptr;
    owns_lock_ = false;

    return ret;
  }

  //unique_lock仍然持有mutex锁
  mutex_type* mutex() noexcept
  {
    return mutex_ptr_outter_;
  }

private:
  mutex_type *mutex_ptr_outter_{ nullptr };
  bool owns_lock_{ false };
};

HelperLibEnd