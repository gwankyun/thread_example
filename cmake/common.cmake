﻿function(to_WIN32_WINNT version result)
  if(version)
    set(ver ${version})
    string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
    string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
    # Check for Windows 10, b/c we'll need to convert to hex 'A'.
    if("${verMajor}" MATCHES "10")
      set(verMajor "A")
      string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
    endif()
    # Remove all remaining '.' characters.
    string(REPLACE "." "" ver ${ver})
    # Prepend each digit with a zero.
    string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
    set(${result} "0x${ver}" PARENT_SCOPE)
  endif()
endfunction()

function(check_package name)
  message(CHECK_START "Looking for ${name}")
  if(${name}_FOUND)
      set(ver ${${name}_VERSION})
      message(CHECK_PASS "found version \"${ver}\"")
  else()
      message(CHECK_FAIL "not found")
  endif()
endfunction()