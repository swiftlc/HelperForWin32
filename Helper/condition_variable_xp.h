#pragma once
#include <windows.h>

typedef struct CONDITION_VARIABLE_XP
{
  unsigned long wait_count_;
  HANDLE event_;
}CONDITION_VARIABLE_XP, *LPCONDITION_VARIABLE_XP;

void InitializeConditionVariableXp(LPCONDITION_VARIABLE_XP p_condition_variable_xp);

bool SleepConditionVariableXp(
  LPCONDITION_VARIABLE_XP p_condition_variable_xp,
  PCRITICAL_SECTION   critical_section,
  DWORD               dwMilliseconds
);

void WakeConditionVariableXp(LPCONDITION_VARIABLE_XP p_condition_variable_xp);

void WakeAllConditionVariableXp(LPCONDITION_VARIABLE_XP p_condition_variable_xp);