#pragma once
#include "common.h"
#include <windows.h>

#include <functional>
#include <filesystem>

HelperLibBegin

namespace fs = std::experimental::filesystem;
void EnumDir(LPCWSTR dir_path, std::function<void(fs::path p)> fn)
{
  if (dir_path == nullptr) return;
  fs::path p{ dir_path };
  if (fs::is_directory(p)) {
    for (auto &fe : fs::directory_iterator(p)) {
      fn(fe);
      if (fs::is_directory(fe.path())) {
        EnumDir(fe.path().c_str(), fn);
      }
    }
  }
}

//TODO...

HelperLibEnd