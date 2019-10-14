#pragma once
#include "common.h"
#include "mutex.h"
#include "unique_lock.h"

HelperLibBegin

//µ¥ÀýÄ£°åÀà
template<typename T>
class SingletonT
  :public NoCopiable
{
  using Lock = unique_lock<mutex>;
protected:
  SingletonT() = default;
public:
  static T* GetInstance()
  {
    //double check
    if (instance_ == nullptr) {
      Lock lock(mtx_);
      if (instance_ == nullptr) {
        instance_ = new T;
      }
    }
    return instance_;
  }

  static T* Instance()
  {
    return GetInstance();
  }

  static void ReleaseInstance()
  {
    if (instance_) {
      Lock lock(mtx_);
      if (instance_) {
        delete instance_;
        instance_ = nullptr;
      }
    }
  }
private:
  static T *instance_;
  static mutex mtx_;
};

template<typename T>
T* SingletonT<T>::instance_ = nullptr;

template<typename T>
mutex SingletonT<T>::mtx_{};

#define HELPERLIBSINGLETON(className) 	friend class helper_lib::SingletonT<className>;

HelperLibEnd


//class A :public SingletonT<A>
//{
//  SINGLETON(A)
//public:
//  void Set(int a) { m_a_ = a; };
//  void Print() { MyLog << m_a_; }
//private:
//  A() = default;
//  int m_a_{ 0 };
//};