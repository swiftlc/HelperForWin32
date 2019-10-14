#include "message_wnd.h"
#include "mutex.h"

HelperLibBegin

MessageWnd::MessageWnd(LPCWSTR wnd_class_name, LPCWSTR wnd_name /*= L""*/)
  :wnd_class_name_(wnd_class_name), wnd_name_(wnd_name)
{

}

bool MessageWnd::CreateWnd(HINSTANCE instance /*= nullptr*/)
{
  if (m_hWnd)return true;

  HWND wnd = CreateMessageWnd(wnd_class_name_.c_str(), wnd_name_.c_str(), instance);

  if (wnd != nullptr) {
    ::SetLastError(ERROR_SUCCESS);
    LONG result =
      ::SetWindowLong(wnd, GWL_USERDATA, reinterpret_cast<LONG>(this));
    if (!result && ::GetLastError() != ERROR_SUCCESS) {
      ::DestroyWindow(wnd);
      return false;
    } else {
      m_hWnd = wnd;
      return true;
    }
  }

  return false;
}

HWND MessageWnd::GetHWnd() const
{
  return m_hWnd;
}

MessageWnd::operator HWND () const
{
  return GetHWnd();
}

bool MessageWnd::Asyn(std::function<void()> task)
{
  return AsyncTask::Post(m_hWnd, task);
}

bool MessageWnd::Asyn(AsyncTask *task)
{
  return AsyncTask::Post(m_hWnd, task);
}

bool helper_lib::MessageWnd::PostMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
  return !!::PostMessage(m_hWnd, msg, wParam, lParam);
}

void helper_lib::MessageWnd::Syn(std::function<void()> task)
{
  ::SendMessage(m_hWnd, kSyncTask, 0, (LPARAM)&task);
}

void helper_lib::MessageWnd::SendMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
  ::SendMessage(m_hWnd, msg, wParam, lParam);
}

MessageWnd::~MessageWnd()
{
  if (m_hWnd) {
    ::DestroyWindow(m_hWnd);
    m_hWnd = nullptr;
  }
}

void MessageWnd::OnWndMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool &handled)
{
  on_message_(hWnd, message, wParam, lParam, handled);
}

HWND MessageWnd::CreateMessageWnd(LPCWSTR class_name, LPCWSTR wnd_name, HINSTANCE instance)
{
  WNDCLASSEX wc{};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = MessageWndProc;
  wc.hInstance = instance;
  wc.lpszClassName = class_name;
  if (!RegisterClassEx(&wc)) {
    return nullptr;
  }

  HWND wnd = CreateWindow(class_name, wnd_name, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, instance, 0);
  if (wnd == nullptr) {
  }

  return wnd;
}

LRESULT CALLBACK MessageWnd::MessageWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if (message == kAsyncTask) {
    ExecuteAsynTask(lParam);
    return FALSE;
  } else if (message == kSyncTask) {
    auto task = reinterpret_cast<std::function<void()>*>(lParam);
    if (task) {
      (*task)();
    }
    return FALSE;
  }

  MessageWnd* message_wnd = reinterpret_cast<MessageWnd*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
  if (message_wnd) {
    bool handled = false;
    message_wnd->OnWndMessage(hWnd, message, wParam, lParam, handled);
    if (handled) return FALSE;
  }

  return ::DefWindowProc(hWnd, message, wParam, lParam);
}

namespace {
  MessageWnd *global_message_wnd{ nullptr };
}

bool InitMessageWnd(LPCWSTR wnd_class_name, LPCWSTR wnd_name /*= L""*/)
{
  static once_flag flag;
  static bool ret = true;
  call_once(flag, [&] {
    global_message_wnd = new MessageWnd(wnd_class_name, wnd_name);
    ret = global_message_wnd->CreateWnd();
  });
  return ret;
}

bool Asyn(std::function<void()> task)
{
  if (global_message_wnd) {
    return global_message_wnd->Asyn(task);
  }
  return false;
}

bool Syn(std::function<void()> task)
{
  if (global_message_wnd) {
    global_message_wnd->Syn(task);
    return true;
  }
  return false;
}

bool helper_lib::PostMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (global_message_wnd) {
    return global_message_wnd->PostMessage(msg, wParam, lParam);
  }
  return false;
}

bool helper_lib::RegisterEventHandler(std::function<bool(HWND, UINT, WPARAM, LPARAM, bool &/*handled*/)> handler)
{
  if (global_message_wnd) {
    global_message_wnd->on_message_ += handler;
  }
  return !!global_message_wnd;
}

bool helper_lib::SendMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (global_message_wnd) {
    global_message_wnd->SendMessage(msg, wParam, lParam);
  }
  return !!global_message_wnd;
}

void UninitMessageWnd()
{
  if (global_message_wnd) {
    static once_flag flag;
    call_once(flag, [&] {
      delete global_message_wnd;
      global_message_wnd = nullptr;
    });
  }
}

HelperLibEnd
