#pragma once
#include "common.h"
#include "asyn_task.h"
#include "event_observer.h"
#include "defer.h"
#include <Windows.h>
#include <iostream>

HelperLibBegin

#define kSyncTask (kAsyncTask + 1)

class MessageWnd
{
public:
  //HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &handled
  Events<HWND, UINT, WPARAM, LPARAM, bool &/*handled*/> on_message_;

  MessageWnd(LPCWSTR wnd_class_name, LPCWSTR wnd_name = L"");

  bool CreateWnd(HINSTANCE instance = nullptr);

  HWND GetHWnd() const;

  operator HWND() const;

  bool Asyn(std::function<void()> task);

  bool Asyn(AsyncTask *task);

  void Syn(std::function<void()> task);

  bool PostMessage(UINT msg, WPARAM wParam, LPARAM lParam);

  void SendMessage(UINT msg, WPARAM wParam, LPARAM lParam);

  virtual ~MessageWnd();

protected:
  virtual void OnWndMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &handled);

  HWND m_hWnd{};  //命名与其他界面框架保持一致
  std::wstring wnd_class_name_{};
  std::wstring wnd_name_{};
private:
  static HWND CreateMessageWnd(LPCWSTR class_name, LPCWSTR wnd_name, HINSTANCE instance);

  static LRESULT CALLBACK MessageWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};


bool InitMessageWnd(LPCWSTR wnd_class_name, LPCWSTR wnd_name = L"");

bool Asyn(std::function<void()> task);

bool Syn(std::function<void()> task);



//HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &handled
bool RegisterEventHandler(std::function<bool(HWND, UINT, WPARAM, LPARAM, bool &/*handled*/)> handler);

bool PostMessage(UINT msg, WPARAM wParam, LPARAM lParam);

bool SendMessage(UINT msg, WPARAM wParam, LPARAM lParam);

void UninitMessageWnd();


#ifndef QuickMessageWndClassName
#define QuickMessageWndInit() \
helper_lib::Defer message_wnd_auto_init_uninit{(helper_lib::InitMessageWnd(L"QuickMessageWndClassName"),[]{helper_lib::UninitMessageWnd();})};
#else
#define QuickMessageWndInit() \
helper_lib::Defer message_wnd_auto_init_uninit{(helper_lib::InitMessageWnd(QuickMessageWndClassName),[]{helper_lib::UninitMessageWnd();})};
#endif

HelperLibEnd