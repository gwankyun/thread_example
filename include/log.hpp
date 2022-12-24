#if HAS_SPDLOG
#  include <spdlog/spdlog.h>
#else
#  include <iostream>
#  define SPDLOG_INFO(x) std::cout << (x) << "\n";
#endif

#define TO_STRING_IMPL(x) #x
#define TO_STRING(x) TO_STRING_IMPL(x)

#define DBG(x) SPDLOG_INFO("{}: {}", TO_STRING(x), x)
