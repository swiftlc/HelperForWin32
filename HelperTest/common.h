#pragma once

#include <windows.h>

#include <tlhelp32.h>
#include <functional>
#include <sstream>

LPWSTR GetCurModuleDir(LPWSTR module_dir, DWORD size);

LPWSTR PathToAbsolute(LPCWSTR path, LPWSTR path_absolute, DWORD size);

HMODULE _LoadLibrary(LPCWSTR dll_path);

void _Wait(DWORD wait_milsec = INFINITE);

void EnumProcess(std::function<bool/*false==>stop*/(PROCESSENTRY32)> enum_fn);

std::wstring _GetCurrentTime();

HANDLE _OpenProcess(DWORD pid);

BOOL DosPathToNtPath(LPCWSTR dos_path, LPWSTR nt_path, DWORD max_sz);

BOOL GetProcessFilePath(DWORD pid, LPWSTR file_path, DWORD max_sz);

class MyConsoleLogger
{
public:
  MyConsoleLogger(MyConsoleLogger const&) = delete;
  MyConsoleLogger& operator=(MyConsoleLogger const&) = delete;

  void operator += (std::wostringstream const&log_record);

  static MyConsoleLogger *GetInstance();
private:
  MyConsoleLogger() = default;
  static MyConsoleLogger* instance;
};

#define MyLog (*MyConsoleLogger::GetInstance()) += std::wostringstream()

#ifdef _WINDOWS
void _AllocConsole();
void _FreeConsole();
#endif // _WINDOWS
