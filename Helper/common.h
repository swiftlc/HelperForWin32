#pragma once

#define HelperLibBegin namespace helper_lib{

#define HelperLibEnd }

HelperLibBegin

class NoCopiable
{
public:
  NoCopiable() = default;
  NoCopiable(NoCopiable const&) = delete;
  NoCopiable(NoCopiable &&) = delete;
  NoCopiable& operator=(NoCopiable const&) = delete;
  NoCopiable& operator=(NoCopiable &&) = delete;
};

HelperLibEnd