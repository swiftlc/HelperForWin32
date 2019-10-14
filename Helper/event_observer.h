#pragma once
#include "common.h"
#include <functional>
#include <list>
#include <algorithm>
#include <memory>

HelperLibBegin

template<typename ...Args>
class Events :
  protected NoCopiable
{
  using EventsWeakPtr = std::weak_ptr<Events>;
  using EventsSharedPtr = std::shared_ptr<Events>;
public:
  //true 事件放行
  using stl_func_type = std::function <bool(Args...)>;

  template<typename Fn>
  void operator+=(Fn &&func)
  {
    observers_.push_back(std::forward<Fn>(func));
  }

  template<typename Fn>
  void operator-=(Fn &&func)
  {
    observers_.push_front(std::forward<Fn>(func));
  }

  template<typename Fn>
  void RegisterLastObserver(Fn &&func)
  {
    operator +=(std::forward<Fn>(func));
  }

  template<typename Fn>
  void RegisterFirstObserver(Fn &&func)
  {
    operator -=(std::forward<Fn>(func));
  }

  void AddLastPipeLine(EventsWeakPtr e_other)
  {
    observers_.push_back([e_other](Args&&...args)
    {
      if (auto sp = e_other.lock()) {
        return (*sp)(std::forward<Args>(args)...);
      } else {
        return true;
      }
    });
  }

  void AddFirstPipeLine(EventsWeakPtr e_other)
  {
    observers_.push_front([e_other](Args&&...args)
    {
      if (auto sp = e_other.lock()) {
        return (*sp)(std::forward<Args>(args)...);
      } else {
        return true;
      }
    });
  }

  void operator+=(EventsWeakPtr e_other) { AddLastPipeLine(e_other); }
  void operator-=(EventsWeakPtr e_other) { AddFirstPipeLine(e_other); }

  void operator+=(EventsSharedPtr e_other) { AddLastPipeLine(e_other); }
  void operator-=(EventsSharedPtr e_other) { AddFirstPipeLine(e_other); }

  template<typename..._Args>
  bool operator()(_Args&&...args)
  {
    return Emit(std::forward<Args>(args)...);
  }

  template<typename..._Args>
  bool Emit(_Args&&...args)
  {
    return std::find_if(observers_.begin(), observers_.end(), [&](stl_func_type const&fn) {
      return !fn(std::forward<Args>(args)...);
    }) == observers_.end();
  }

private:
  std::list<stl_func_type> observers_;
};

template<typename ...Args>
using EventsPtr = std::shared_ptr<Events<Args...>>;

template<typename ...Args>
inline decltype(auto) MakeEvents()
{
  return std::make_shared<Events<Args...>>();
}

HelperLibEnd