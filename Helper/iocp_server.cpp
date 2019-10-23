#include "iocp_server.h"
#include <algorithm>

HelperLibBegin

IocpServer::~IocpServer()
{
  if (completion_port != INVALID_HANDLE_VALUE) {
    ::CloseHandle(completion_port);
    completion_port = INVALID_HANDLE_VALUE;
  }
}

bool IocpServer::Init(WORD asyn_task_count /*= 0*/, WORD event_handler_count /*= 1*/)
{
  completion_port = ::CreateIoCompletionPort(
    INVALID_HANDLE_VALUE,
    NULL,
    0,
    asyn_task_count == 0 ? DefaultAsynTaskCount() : asyn_task_count);
  if (completion_port == INVALID_HANDLE_VALUE) return false;
  latch_.reset(new CountDownLatch{ event_handler_count });
  event_handler_count = event_handler_count == 0 ? DefaultAsynTaskCount() : event_handler_count;
  for (WORD i = 0; i < event_handler_count; i++) {
    event_handlers_.push_back(thread{ std::bind(&IocpServer::ThreadFunc,this) });
  }
  return true;
}

void IocpServer::Quit()
{
  if (completion_port != INVALID_HANDLE_VALUE) {
    PostExitSignal();
    latch_->Wait();

    //线程资源清理
    std::for_each(event_handlers_.begin(), event_handlers_.end(),
      [](auto& t) {if (t.joinable())t.join(); });
    event_handlers_.clear();
    ::CloseHandle(completion_port);
    completion_port = INVALID_HANDLE_VALUE;
  }
}

void helper_lib::IocpServer::QuitA(DWORD milli_second)
{
  if (completion_port != INVALID_HANDLE_VALUE) {
    PostExitSignal();

    if (latch_->AWait(milli_second * 1ms) == WaitReturn::Success) {
      //线程资源清理
      std::for_each(event_handlers_.begin(), event_handlers_.end(),
        [](auto& t) {if (t.joinable())t.join(); });
    }
    event_handlers_.clear();
    ::CloseHandle(completion_port);
    completion_port = INVALID_HANDLE_VALUE;
  }
}

bool IocpServer::AssociateHandle(HANDLE handle, ULONG_PTR context)
{
  if (completion_port == INVALID_HANDLE_VALUE) {
    return false;
  }
  return ::CreateIoCompletionPort(handle, completion_port, context, 0) != INVALID_HANDLE_VALUE;
}

void IocpServer::PostExitSignal()
{
  if (completion_port != INVALID_HANDLE_VALUE) {
    auto t_count = event_handlers_.size();
    for (decltype(t_count) i = 0; i < t_count; i++) {
      ::PostQueuedCompletionStatus(completion_port, SPECIAL_CODE, SPECIAL_CODE, (LPOVERLAPPED)SPECIAL_CODE);
    }
  }
}

WORD IocpServer::DefaultAsynTaskCount()
{
  return thread::hardware_concurrency() * 2 + 2;
}

void helper_lib::IocpServer::ThreadFunc()
{
  DWORD bytes_transfer{};
  ULONG_PTR context{};
  LPOVERLAPPED ol{};
  for (;;) {
    BOOL bRet = GetQueuedCompletionStatus(completion_port, &bytes_transfer, &context, &ol, INFINITE);
    if (SPECIAL_CODE == bytes_transfer && SPECIAL_CODE == (DWORD)ol) {
      //优雅退出
      if (SPECIAL_CODE == (DWORD)context) {
        break;
      }
      else {
        //异步任务
        auto t = (_Task*)context;
        if (t != nullptr) {
          (*t)();
          delete t;
        }
        continue;
      }
    }
    if (!bRet) {  //发生错误
      OnError(::GetLastError(), context, ol);
    }
    else {
      OnComplete(bytes_transfer, context, ol);
    }
  }

  latch_->CountDown();
}

HelperLibEnd

