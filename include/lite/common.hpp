#pragma once
#include <Windows.h>

#ifndef NOEXCEPT
#  if __cplusplus >= 201103L
#    define NOEXCEPT noexcept
#  else
#    define NOEXCEPT
#  endif
#endif

namespace lite
{
    inline DWORD wait(HANDLE& _handle, unsigned long _milliseconds = INFINITE)
    {
        return WaitForSingleObject(_handle, _milliseconds);
    }
}
