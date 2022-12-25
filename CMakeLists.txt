# cmake最低版本
cmake_minimum_required(VERSION 3.23)

# 項目名
set(project thread_example)
project(${project})

include(CMakePrintHelpers)

# 啟用IDE目錄功能
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 輸出目錄
set(source_dir ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${source_dir}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${source_dir}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${source_dir}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${source_dir}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${source_dir}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${source_dir}/lib)

if(WIN32)
  function(get_WIN32_WINNT version)
    if(CMAKE_SYSTEM_VERSION)
      set(ver ${CMAKE_SYSTEM_VERSION})
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
      set(${version} "0x${ver}" PARENT_SCOPE)
    endif()
  endfunction()

  get_WIN32_WINNT(ver)
  cmake_print_variables(CMAKE_SYSTEM_VERSION)
  cmake_print_variables(ver)
  add_compile_definitions(_WIN32_WINNT=${ver})
endif()

find_package(spdlog CONFIG) # 可選

set(main main)
add_executable(${main})
target_sources(${main} PRIVATE
  src/main.cpp
  include/common.hpp
)
target_include_directories(${main} PRIVATE include)
target_compile_features(${main} PUBLIC cxx_std_20)
target_link_libraries(${main} PRIVATE
  $<TARGET_NAME_IF_EXISTS:spdlog::spdlog>
)
target_compile_definitions(${main} PRIVATE
  HAS_SPDLOG=$<TARGET_EXISTS:spdlog::spdlog>
)

set(windows windows)
add_executable(${windows})
target_sources(${windows} PRIVATE
  include/lite/thread.hpp
  include/lite/mutex.hpp
  include/lite/event.hpp
  include/lite/future.hpp
  src/windows.cpp
)
target_include_directories(${windows} PRIVATE include)
target_link_libraries(${windows} PRIVATE
  $<TARGET_NAME_IF_EXISTS:spdlog::spdlog>
)
target_compile_definitions(${windows} PRIVATE
  HAS_SPDLOG=$<TARGET_EXISTS:spdlog::spdlog>
)
