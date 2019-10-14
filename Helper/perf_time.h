#pragma once
#include "common.h"
#include <windows.h>

HelperLibBegin

class PerfTime
  :protected NoCopiable
{
public:
  PerfTime()
  {
    GetPerfTimeFreq();
    Reset();
  }

  void Reset()
  {
    ::QueryPerformanceCounter(&begin_);
  }

  UINT64 GetPerfTimeFreq()
  {
    ::QueryPerformanceFrequency(&freq_);
    return freq_.QuadPart;
  }

  UINT64 TimeCastMicroSec()
  {
    LARGE_INTEGER end{};
    ::QueryPerformanceCounter(&end);
    return (UINT64)((end.QuadPart - begin_.QuadPart) * 1e6 / freq_.QuadPart);
  }
private:
  LARGE_INTEGER begin_{};
  LARGE_INTEGER freq_{};
};

#define BeginPerf() PerfTime perf_time{}

#define BeginPerf2() perf_time.Reset()

#define EndPerf(time_cast) UINT64 time_cast = perf_time.TimeCastMicroSec()

#define EndPerf2(time_cast) time_cast = perf_time.TimeCastMicroSec()

HelperLibEnd