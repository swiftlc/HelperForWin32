#include "thread.h"

HelperLibBegin


thread::thread(thread&& other) noexcept
  :native_handle_(other.native_handle_),
  thread_id_(other.thread_id_)
{
  other.native_handle_ = nullptr;
  other.thread_id_ = 0;
}

thread& thread::operator=(thread&& other) noexcept
{
  if (joinable()) {
    ::TerminateThread(native_handle_, 0);
  }

  native_handle_ = other.native_handle_;
  thread_id_ = other.thread_id_;
  other.native_handle_ = nullptr;
  other.thread_id_ = 0;

  return *this;
}

thread::~thread()
{
  if (joinable()) {
    ::TerminateThread(native_handle_, 0);
    ::CloseHandle(native_handle_);
  }
}

thread::id thread::get_id()const noexcept
{
  return thread_id_;
}

volatile HANDLE* thread::native_handle()
{
  return &native_handle_;
}

bool thread::joinable() const noexcept
{
  return native_handle_ != nullptr && native_handle_ != INVALID_HANDLE_VALUE;
}

void thread::join()
{
  if (!joinable()) {
    throw std::runtime_error("thread unjoinable");
  } else {
    ::WaitForSingleObject(native_handle_, INFINITE);
    ::CloseHandle(native_handle_);
    native_handle_ = nullptr;
    thread_id_ = 0;
  }
}

bool thread::join_for(DWORD mill_second)
{
  if (!joinable()) {
    throw std::runtime_error("thread unjoinable");
  } else {
    DWORD wait_ret = ::WaitForSingleObject(native_handle_, mill_second);
    if (wait_ret == WAIT_OBJECT_0) {  //wait success
      ::CloseHandle(native_handle_);
      native_handle_ = nullptr;
      thread_id_ = 0;
      return true;
    }
  }
  return false;
}

void thread::detach()
{
  if (!joinable()) {
    throw std::runtime_error("thread unjoinable");
  } else {
    ::CloseHandle(native_handle_);
    native_handle_ = nullptr;
    thread_id_ = 0;
  }
}

unsigned int thread::hardware_concurrency_{ 0 };

unsigned __stdcall thread::ThreadProc(void *param)
{
  auto p = reinterpret_cast<std::function<void()>*>(param);
  (*p)();
  delete p;
  return 0;
}

unsigned int thread::hardware_concurrency() noexcept
{
  if (hardware_concurrency_ > 0) {
    return hardware_concurrency_;
  }
  SYSTEM_INFO sys_info{};
  ::GetSystemInfo(&sys_info);
  hardware_concurrency_ = sys_info.dwNumberOfProcessors;
  return hardware_concurrency_;
}

HelperLibEnd