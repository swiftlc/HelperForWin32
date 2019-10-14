#include <windows.h>
#include "common.h"

#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")

#include <strsafe.h>

#define PSAPI_VERSION 1
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")

#define MainPathRELATIVE L"."

LPWSTR GetCurModuleDir(LPWSTR module_dir, DWORD size)
{
  ::ZeroMemory(module_dir, size);
  ::GetModuleFileNameW(NULL, module_dir, MAX_PATH);
  ::PathRemoveFileSpecW(module_dir);
  ::PathCombine(module_dir, module_dir, MainPathRELATIVE);
  return module_dir;
}

LPWSTR PathToAbsolute(LPCWSTR path, LPWSTR path_absolute, DWORD size)
{
  if (path == NULL) return path_absolute;

  if (::PathIsRelativeW(path)) {
    WCHAR module_dir[MAX_PATH]{};
    PathCombineW(GetCurModuleDir(module_dir, MAX_PATH), module_dir, path);
    StringCchCopyW(path_absolute, size, module_dir);
  } else {
    StringCchCopyW(path_absolute, size, path);
  }
  return path_absolute;
}

HMODULE _LoadLibrary(LPCWSTR dll_path)
{
  WCHAR absolute_dll_path[MAX_PATH]{};
  return ::LoadLibraryW(PathToAbsolute(dll_path, absolute_dll_path, MAX_PATH));
}

void _Wait(DWORD wait_milsec /*= INFINITE*/)
{
  HANDLE event = ::CreateEvent(NULL, TRUE, FALSE, NULL);
  ::WaitForSingleObject(event, wait_milsec);
  ::CloseHandle(event);
}

void EnumProcess(std::function<bool/*false==>stop*/(PROCESSENTRY32)> enum_fn)
{
  HANDLE hProcSnapShot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcSnapShot != INVALID_HANDLE_VALUE) {
    PROCESSENTRY32 pe32 = { 0 };
    pe32.dwSize = sizeof(pe32);
    BOOL bFind = ::Process32First(hProcSnapShot, &pe32);
    while (bFind) {
      if (!enum_fn(pe32))break;
      bFind = ::Process32Next(hProcSnapShot, &pe32);
    }
    ::CloseHandle(hProcSnapShot);
  }
}

std::wstring _GetCurrentTime()
{
  SYSTEMTIME sys;
  GetLocalTime(&sys);
  WCHAR buf[128]{};
  ::StringCchPrintfW(buf, 128, L"%02d:%02d:%02d.%03d\t:", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
  return buf;
}

HANDLE _OpenProcess(DWORD pid)
{
  HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  return hProcess == NULL ?
    ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid) :
    hProcess;
}

BOOL DosPathToNtPath(LPCWSTR dos_path, LPWSTR nt_path, DWORD max_sz)
{
  TCHAR			szDriveStr[500];
  TCHAR			szDrive[3];
  TCHAR			szDevName[100];
  INT				cchDevName;
  INT				i;

  if (!dos_path || !nt_path)
    return FALSE;

  if (::GetLogicalDriveStrings(sizeof(szDriveStr), szDriveStr))
  {
    for (i = 0; szDriveStr[i]; i += 4)
    {
      szDrive[0] = szDriveStr[i];
      szDrive[1] = szDriveStr[i + 1];
      szDrive[2] = '\0';

      if (!::QueryDosDevice(szDrive, szDevName, 100))
        continue;

      cchDevName = lstrlen(szDevName);
      if (wcsncmp(dos_path, szDevName, cchDevName) == 0)
      {
        StringCchCopy(nt_path, max_sz, szDrive);
        StringCchCat(nt_path, max_sz, dos_path + cchDevName);
        return TRUE;
      }
    }
  }
  return FALSE;
}

BOOL GetProcessFilePath(DWORD pid, LPWSTR file_path, DWORD max_sz)
{
  if (file_path == NULL) return FALSE;

  HANDLE hProcess = _OpenProcess(pid);
  if (hProcess == NULL) return FALSE;

  TCHAR szCurrentProcessName[MAX_PATH];
  ::ZeroMemory(szCurrentProcessName, _countof(szCurrentProcessName));

  BOOL bResult = FALSE;
  if (::GetModuleFileNameEx(hProcess, NULL, szCurrentProcessName, _countof(szCurrentProcessName))) {
    bResult = TRUE;
    StringCchCopy(file_path, max_sz, szCurrentProcessName);
  } else if (::GetProcessImageFileName(hProcess, szCurrentProcessName, _countof(szCurrentProcessName))) {
    bResult = DosPathToNtPath(szCurrentProcessName, file_path, max_sz);
  }

  CloseHandle(hProcess);
  return bResult;
}

#ifdef _WINDOWS
HANDLE g_output_handle = INVALID_HANDLE_VALUE;

void _AllocConsole()
{
  ::AllocConsole();
  g_output_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
}
void _FreeConsole()
{
  ::FreeConsole();
  g_output_handle = INVALID_HANDLE_VALUE;
}
#else
#include <iostream>
#endif

void MyConsoleLogger::operator += (std::wostringstream const&log_record)
{
  std::wstring str = std::move(log_record.str());

#ifdef _WINDOWS
  if (g_output_handle != INVALID_HANDLE_VALUE) {
    str = _GetCurrentTime() + str + L"\n";
    WriteConsoleW(g_output_handle, str.c_str(), str.length(), NULL, NULL);
}
#else
  str = _GetCurrentTime() + str;
  std::wcout << str.c_str() << std::endl;
#endif
}

MyConsoleLogger * MyConsoleLogger::GetInstance()
{
  if (instance == nullptr) {
    instance = new MyConsoleLogger;
  }
  return instance;
}

MyConsoleLogger* MyConsoleLogger::instance = nullptr;
