#include "condition_variable_xp.h"

void InitializeConditionVariableXp(LPCONDITION_VARIABLE_XP p_condition_variable_xp)
{
  p_condition_variable_xp->wait_count_ = 0;
  p_condition_variable_xp->event_ = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

bool SleepConditionVariableXp(
  LPCONDITION_VARIABLE_XP p_condition_variable_xp,
  PCRITICAL_SECTION   critical_section,
  DWORD               dwMilliseconds
)
{
  p_condition_variable_xp->wait_count_++;
  ::LeaveCriticalSection(critical_section);
  auto ret = ::WaitForSingleObject(p_condition_variable_xp->event_, dwMilliseconds);
  ::EnterCriticalSection(critical_section);
  p_condition_variable_xp->wait_count_--;
  return ret == WAIT_OBJECT_0;
}

void WakeConditionVariableXp(LPCONDITION_VARIABLE_XP p_condition_variable_xp)
{
  ::SetEvent(p_condition_variable_xp->event_);
}

void WakeAllConditionVariableXp(LPCONDITION_VARIABLE_XP p_condition_variable_xp)
{
  auto count = p_condition_variable_xp->wait_count_;
  while (count--) {
    WakeConditionVariableXp(p_condition_variable_xp);
  }
}