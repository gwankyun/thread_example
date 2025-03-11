# cmake最低版本
cmake_minimum_required(VERSION 3.31)

# 項目名
set(project thread_example)
project(${project})

include(CMakePrintHelpers)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(common)

# 輸出目錄
set(source_dir ${CMAKE_CURRENT_SOURCE_DIR})

# 包含模塊
include(CMakePrintHelpers) # cmake_print_variables

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${source_dir}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${source_dir}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${source_dir}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${source_dir}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${source_dir}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${source_dir}/lib)

to_WIN32_WINNT(${CMAKE_SYSTEM_VERSION} ver)
cmake_print_variables(CMAKE_SYSTEM_VERSION)
cmake_print_variables(ver)
add_compile_definitions("$<$<BOOL:WIN32>:_WIN32_WINNT=${ver}")

find_package(spdlog CONFIG) # 可選
check_package(spdlog)

set(common common)
add_library(${common} INTERFACE)
target_sources(${common} INTERFACE
  include/common.hpp
)
target_include_directories(${common} INTERFACE include)
target_link_libraries(${common} INTERFACE
  $<TARGET_NAME_IF_EXISTS:spdlog::spdlog>
)
target_compile_definitions(${common} INTERFACE
  HAS_SPDLOG=$<TARGET_EXISTS:spdlog::spdlog>
)

set(main main)
add_executable(${main})
target_sources(${main} PRIVATE
  src/main.cpp
)
target_compile_features(${main} PUBLIC cxx_std_20)
target_link_libraries(${main} PRIVATE
  ${common}
)

set(example_list detach_example atomic_example barrier_example latch_example
  semaphore_example jthread_example async_example packaged_task_example
  promise_example condition_variable_notify_all_example condition_variable_notify_one_example
  mutex_example thread_example
)

set(detach_example detach_example)
add_executable(${detach_example})

set(atomic_example atomic_example)
add_executable(${atomic_example})

set(barrier_example barrier_example)
add_executable(${barrier_example})

set(latch_example latch_example)
add_executable(${latch_example})

set(semaphore_example semaphore_example)
add_executable(${semaphore_example})

set(jthread_example jthread_example)
add_executable(${jthread_example})

set(async_example async_example)
add_executable(${async_example})

set(packaged_task_example packaged_task_example)
add_executable(${packaged_task_example})

set(promise_example promise_example)
add_executable(${promise_example})

set(condition_variable_notify_all_example condition_variable_notify_all_example)
add_executable(${condition_variable_notify_all_example})

set(condition_variable_notify_one_example condition_variable_notify_one_example)
add_executable(${condition_variable_notify_one_example})

set(mutex_example mutex_example)
add_executable(${mutex_example})

set(thread_example thread_example)
add_executable(${thread_example})

foreach(example IN LISTS example_list)
  cmake_print_variables(example)
  target_sources(${example} PRIVATE
    src/${example}.cpp
  )
  target_compile_features(${example} PUBLIC cxx_std_20)
  target_link_libraries(${example} PRIVATE
    ${common}
  )
endforeach()

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()
find_package(Boost REQUIRED)
check_package(Boost)

find_package(Catch2 3 REQUIRED)
check_package(Catch2)

set(test test)
add_executable(${test})
target_sources(${test} PRIVATE
  src/test.cpp
)
target_compile_features(${test} PUBLIC cxx_std_23)
target_link_libraries(${test} PRIVATE
  spdlog::spdlog
  Catch2::Catch2
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
target_include_directories(${windows} PRIVATE include)# ${Boost_INCLUDE_DIRS})
target_link_libraries(${windows} PRIVATE
  $<TARGET_NAME_IF_EXISTS:spdlog::spdlog>
  Boost::headers
)
target_compile_definitions(${windows} PRIVATE
  HAS_SPDLOG=$<TARGET_EXISTS:spdlog::spdlog>
)
