#pragma once
#include "common.h"
#include<string>
#include <windows.h>

HelperLibBegin

//CP_ACP local GBK
//CP_UTF8 utf8

inline std::wstring ToWString(const char* const src, UINT code_page = CP_ACP)
{
  if (src == NULL) return {};
  size_t len = ::strlen(src);
  if (len == 0) return {};
  int len_rel = ::MultiByteToWideChar(code_page, 0, src, -1, NULL, 0);
  std::wstring str(len_rel, 0);
  ::MultiByteToWideChar(code_page, 0, src, -1, &str[0], len_rel);
  str.resize(len_rel - 1);
  return str;
}

inline std::wstring ToWString(std::string const& src, UINT code_page = CP_ACP)
{
  if (src.size() == 0) return {};
  int len_rel = ::MultiByteToWideChar(code_page, 0, src.c_str(), src.size(), NULL, 0);
  std::wstring str(len_rel, 0);
  ::MultiByteToWideChar(code_page, 0, src.c_str(), src.size(), &str[0], len_rel);
  return str;
}

inline std::string ToString(const wchar_t* const src, UINT code_page = CP_ACP)
{
  if (src == NULL) return {};
  size_t len = ::wcslen(src);
  if (len == 0) return {};
  int len_rel = ::WideCharToMultiByte(code_page, 0, src, -1, NULL, 0, NULL, NULL);
  std::string str(len_rel, 0);
  ::WideCharToMultiByte(code_page, 0, src, -1, &str[0], len_rel, NULL, NULL);
  str.resize(len_rel - 1);
  return str;
}

inline std::string ToString(std::wstring const& src, UINT code_page = CP_ACP)
{
  if (src.size() == 0) return {};
  int len_rel = ::WideCharToMultiByte(code_page, 0, src.c_str(), src.size(), NULL, 0, NULL, NULL);
  std::string str(len_rel, 0);
  ::WideCharToMultiByte(code_page, 0, src.c_str(), src.size(), &str[0], len_rel, NULL, NULL);
  return str;
}

inline std::string ToString(const char* const src, UINT code_page_from, UINT code_page_to)
{
  return ToString(ToWString(src, code_page_from), code_page_to);
}

inline std::string ToString(std::string const& src, UINT code_page_from, UINT code_page_to)
{
  return ToString(ToWString(src, code_page_from), code_page_to);
}

inline std::string ToString(const char* const src, bool utf8_to_ascii = true)
{
  return ToString(src, utf8_to_ascii ? CP_UTF8 : CP_ACP, utf8_to_ascii ? CP_ACP : CP_UTF8);
}

inline std::string ToString(std::string const& src, bool utf8_to_ascii = true)
{
  return ToString(src, utf8_to_ascii ? CP_UTF8 : CP_ACP, utf8_to_ascii ? CP_ACP : CP_UTF8);
}

HelperLibEnd


//std::wcout.imbue(std::locale("chs"));
//
////gbk default
//std::string s1 = ToString(L"我爱祖国");
//std::wstring s2 = ToWString(s1);
//MyLog << s2.c_str();
//
////utf 8
//std::string s3 = ToString(L"我爱祖国", CP_UTF8);
//std::wstring s4 = ToWString(s3, CP_UTF8);
//MyLog << s4.c_str();