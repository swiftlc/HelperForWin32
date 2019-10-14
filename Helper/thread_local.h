#pragma once
#include "common.h"
#include <unordered_map>
#include "thread.h"
#include "mutex.h"
#include "unique_lock.h"

HelperLibBegin

template<typename T>
class ThreadLocalT
  :protected NoCopiable
{
  using Lock = unique_lock<mutex>;
public:
  ThreadLocalT() = default;

  ~ThreadLocalT()
  {
    stored_data_.clear();
  }

  void Set(T const& value)
  {
    auto const& current_thread_id = GetCurrentThreadID();
    Lock lk{ mtx_ };
    auto it_find = stored_data_.find(current_thread_id);
    if (it_find != stored_data_.end()) {
      stored_data_[current_thread_id] = value;
    } else {
      stored_data_.insert({ current_thread_id,value });
    }
  }

  void Remove()
  {
    auto const& current_thread_id = GetCurrentThreadID();
    Lock lk{ mtx_ };
    auto it_find = stored_data_.find(current_thread_id);
    if (it_find != stored_data_.end()) {
      stored_data_.erase(it_find);
    }
  }

  operator T() const
  {
    return const_cast<ThreadLocalT*>(this)->Get();
  }

  ThreadLocalT& operator=(T const&v)
  {
    Set(v);
    return *this;
  }

  T Get(T const&default_value = {}, bool set_if_not_exist = false)
  {
    auto const& current_thread_id = GetCurrentThreadID();
    Lock lk{ mtx_ };
    auto it_find = stored_data_.find(current_thread_id);
    if (it_find != stored_data_.end()) {
      return it_find->second;
    }
    if (set_if_not_exist) {
      stored_data_.insert({ current_thread_id,default_value });
    }
    return default_value;
  }

  //definite set def_value if not exist value
  //在确保线程安全的环境下使用
  T& GetRef(T const&default_value = {})
  {
    auto const& current_thread_id = GetCurrentThreadID();
    auto it_find = stored_data_.find(current_thread_id);
    if (it_find != stored_data_.end()) {
      return it_find->second;
    }
    stored_data_.insert({ current_thread_id,default_value });

    return stored_data_[current_thread_id];
  }

  inline bool IsExistData() const
  {
    Lock lk{ mtx_ };
    return stored_data_.find(GetCurrentThreadID()) != stored_data_.end();
  }

  std::unordered_map<thread::id, T> GetStoredData()
  {
    return stored_data_;
  }

private:
  inline thread::id GetCurrentThreadID() const
  {
    return (thread::id)::GetCurrentThreadId();
  }

private:
  mutex mtx_{};
  std::unordered_map<thread::id, T> stored_data_{};
};

HelperLibEnd




//std::map<int, int> mp{};
////测试多线程写Map线程安全的问题
//std::thread{ [&mp] {
//  int count = 0;
//  for (;;) {
//    count += 2;
//    mp.insert({ count,count });
//    if (count > 1000)break;
//  }
//}
//}.detach();

//int count = 1;
//for (;;) {
//  count += 2;
//  mp.insert({ count,count });
//  if (count > 1000)break;
//}

//MyLog << "Over";
//_Wait();