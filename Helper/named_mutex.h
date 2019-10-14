#pragma once
#include "common.h"
#include "windows.h"
#include "ensure_clean_up.h"
#include "defer.h"

HelperLibBegin

class NamedMutex
{
public:
  inline static bool NamedMutexExist(LPCTSTR mutex_name)
  {
    CEnsureCloseHandle tmp = ::OpenMutex(MUTEX_ALL_ACCESS, FALSE, mutex_name);
    return tmp.IsValid() && ::GetLastError() != ERROR_FILE_NOT_FOUND;
  }

  bool Create(LPCTSTR mutex_name)
  {
    mutex_ = ::CreateMutex(NULL, FALSE, mutex_name);
    return mutex_.IsValid();
  }

  bool Lock()
  {
    return ::WaitForSingleObject(mutex_, INFINITE) == WAIT_OBJECT_0;
  }

  bool Unlock()
  {
    return !!::ReleaseMutex(mutex_);
  }

protected:
  CEnsureCloseHandle mutex_{};
};

#define NamedMutexAutoLock(mtx) DEFER((mtx.Lock(), [&] {mtx.Unlock(); }))
#define NamedMutexAutoLock_SUFFIX(suffix,mtx) DEFER_SUFFIX(suffix,(mtx.Lock(), [&] {mtx.Unlock(); }))

HelperLibEnd



//NamedMutex nm;
//MyLog << NamedMutex::NamedMutexExist(L"Hello");
//MyLog << nm.Create(L"Hello");
//MyLog << NamedMutex::NamedMutexExist(L"Hello");
//MyLog << L"before lock";
//nm.Lock();
//MyLog << L"locked";
//Sleep(5000);
//nm.Unlock();
//MyLog << L"after lock";