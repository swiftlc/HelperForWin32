#include "iocp_wrapper.h"

HelperLibBegin

bool IocpWrapper::Init(WORD asyn_task_count /*= 0*/, WORD event_handler_count /*= 1*/)
{
  completion_port = ::CreateIoCompletionPort(
    INVALID_HANDLE_VALUE,
    NULL,
    0,
    asyn_task_count == 0 ? DefaultAsynTaskCount() : asyn_task_count);
  if (completion_port == INVALID_HANDLE_VALUE) return false;

  event_handler_count = event_handler_count == 0 ? DefaultAsynTaskCount() : event_handler_count;
  for (WORD i = 0; i < event_handler_count; i++) {
    event_handlers_.push_back(
      thread{ [this] {
                DWORD bytes_transfer{};
                ULONG_PTR context{};
                LPOVERLAPPED ol{};
                for (;;) {
                  BOOL bRet = GetQueuedCompletionStatus(completion_port, &bytes_transfer, &context, &ol, INFINITE);
                  if (SPECIAL_CODE == bytes_transfer && SPECIAL_CODE == (DWORD)ol) {
                    //优雅退出
                    if (SPECIAL_CODE == (DWORD)context) {
                    break;
                    } else {
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
                    OnError(bytes_transfer, context, ol);
                  } else {
                    OnComplete(bytes_transfer, context, ol);
                  }
                }
        } });
  }
  return true;
}

void IocpWrapper::Quit()
{
  if (completion_port != INVALID_HANDLE_VALUE) {
    auto t_count = event_handlers_.size();
    for (decltype(t_count) i = 0; i < t_count; i++) {
      PostExitSignal();
    }
    for (decltype(t_count) i = 0; i < t_count; i++) {
      if (event_handlers_[i].joinable()) {
        event_handlers_[i].join();
      }
    }
    event_handlers_.clear();
    completion_port = INVALID_HANDLE_VALUE;
  }
}

bool IocpWrapper::AssociateHandle(HANDLE handle, ULONG_PTR context)
{
  if (completion_port == INVALID_HANDLE_VALUE) {
    return false;
  }
  return ::CreateIoCompletionPort(handle, completion_port, context, 0) != INVALID_HANDLE_VALUE;
}

void IocpWrapper::PostExitSignal()
{
  if (completion_port != INVALID_HANDLE_VALUE) {
    ::PostQueuedCompletionStatus(completion_port, SPECIAL_CODE, SPECIAL_CODE, (LPOVERLAPPED)SPECIAL_CODE);
  }
}

void IocpWrapper::OnError(DWORD bytes_transfer, ULONG_PTR context, LPOVERLAPPED ol)
{

}

void IocpWrapper::OnComplete(DWORD bytes_transfer, ULONG_PTR context, LPOVERLAPPED ol)
{

}

WORD IocpWrapper::DefaultAsynTaskCount()
{
  return thread::hardware_concurrency() * 2 + 2;
}

HelperLibEnd


