#pragma once
#include "common.h"
#include <functional>

HelperLibBegin

class Defer
{
  using Operation = std::function<void()>;
public:
  Defer(Operation op)
    :op_(op)
  {
  }

  virtual ~Defer()
  {
    if (op_) {
      op_();
    }
  }

protected:
  Operation op_{};
};

#define DEFER(fn) helper_lib::Defer __defer(fn)
#define DEFER_SUFFIX(suffix,fn) helper_lib::Defer __defer##suffix(fn)
HelperLibEnd