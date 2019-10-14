#pragma once

//1.不支持wstring类型成员变量
//2.结构体必须存在默认构造
//3.线性容器仅支持vector
//4.支持map结构

#include "x2struct/x2struct.hpp"
#include "string.h"

HelperLibBegin

#define QuickJson(...) \
std::string ToJson() const \
{\
return x2struct::X::tojson(*this);\
}\
std::wstring ToJsonW(UINT code_page = CP_UTF8) const\
{ \
return helper_lib::ToWString(ToJson(),code_page);\
}\
bool FromJson(std::string const&json_str)\
{\
try\
{\
  return x2struct::X::loadjson(json_str, *this, false); \
} catch (const std::exception&)\
{\
    return false;\
}\
}\
bool FromJsonW(std::wstring const&json_str,UINT code_page = CP_UTF8)\
{\
  return FromJson(helper_lib::ToString(json_str, code_page)); \
}\
XTOSTRUCT(__VA_ARGS__);


template <typename T>
inline bool LoadJson(std::string const&json_str, T &t)
{
  try
  {
    return x2struct::X::loadjson(json_str, t, false);
  } catch (...)
  {
    return false;
  }
}

template <typename T>
inline bool LoadJsonW(std::wstring const&json_str, T &t, UINT code_page = CP_UTF8)
{
  try
  {
    return x2struct::X::loadjson(ToString(json_str, code_page), t, false);
  } catch (...)
  {
    return false;
  }
}

template <typename T>
inline std::string ToJson(T const&t)
{
  try
  {
    return x2struct::X::tojson(t);
  } catch (...)
  {
    return {};
  }
}

template <typename T>
inline std::wstring ToJsonW(T const&t, UINT code_page = CP_UTF8)
{
  try
  {
    return ToWString(x2struct::X::tojson(t), code_page);
  } catch (...)
  {
    return {};
  }
}

HelperLibEnd

