#pragma once
#include "common.h"
#include "named_mutex.h"
#include "ensure_clean_up.h"
#include <tchar.h>
#include <functional>
#include <stdio.h>


HelperLibBegin

class ShareMemory
{
public:
  bool Create(LPCTSTR file_map_name, DWORD size_hight, DWORD size_low)
  {
    if (file_map_name == NULL || _tcsclen(file_map_name) == 0 || (size_low == 0 && size_hight == 0))  return false;

    file_map_ = ::CreateFileMapping(
      INVALID_HANDLE_VALUE,
      NULL,
      PAGE_READWRITE,
      size_hight,
      size_low,
      file_map_name
    );

    if (file_map_.IsInvalid())  return false;

    return MapFileAndCreateMutex(file_map_name);
  }

  bool Open(LPCTSTR file_map_name)
  {
    file_map_ = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, file_map_name);

    if (file_map_.IsInvalid()) return false;

    return MapFileAndCreateMutex(file_map_name);
  }

  //同步
  void Use(std::function<void(PVOID)> user_fn)
  {
    if (user_fn && share_mem_.IsValid()) {
      NamedMutexAutoLock(mtx_);
      user_fn(share_mem_);
    }
  }

  //无锁保证
  operator char*()
  {
    return (char *)(PVOID)share_mem_;
  }

  //无锁保证
  operator const char*() const
  {
    return (const char *)(PVOID)const_cast<ShareMemory*>(this)->share_mem_;
  }

  //无锁保证
  operator const char*()
  {
    return (const char *)(PVOID)share_mem_;
  }
private:
  bool MapFileAndCreateMutex(LPCTSTR file_map_name)
  {
    share_mem_ = ::MapViewOfFile(file_map_, FILE_MAP_ALL_ACCESS, 0, 0, 0);

    if (share_mem_.IsInvalid()) return false;

    TCHAR mutex_name[MAX_PATH]{};
    _stprintf_s(mutex_name, _T("%s_mutex"), file_map_name);
    return mtx_.Create(mutex_name);
  }

protected:
  CEnsureCloseHandle file_map_{};
  NamedMutex mtx_{};
  CEnsureUnmapViewOfFile  share_mem_{};
};

HelperLibEnd




//NamedMutex nm;
//ShareMemory sm;
//if (nm.NamedMutexExist(L"Hello")) {
//  sm.Open(L"HAHA");
//  sm.Use([](LPVOID data) {
//    MyLog << (char*)data;
//  });
//} else {
//  nm.Create(L"Hello");
//  sm.Create(L"HAHA", 0, 10);
//  sm.Use([](LPVOID data) {
//    sprintf((char*)data, "hello");
//  });
//}
