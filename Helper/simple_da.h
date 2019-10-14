#pragma once
#include "common.h"
#include <list>
#include <algorithm>

HelperLibBegin

template<typename Type>
class SimpleDA
{
public:
  void AppendData(Type data)
  {
    data_list_.push_back(data);
  }

  Type Min()
  {
    return *std::min_element(data_list_.begin(), data_list_.end());
  }

  Type Max()
  {
    return *std::max_element(data_list_.begin(), data_list_.end());
  }

  void RemoveAll()
  {
    data_list_.clear();
  }
private:
  std::list<Type> data_list_{};
};

HelperLibEnd