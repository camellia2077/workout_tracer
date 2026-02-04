// common/version.hpp

#ifndef COMMON_VERSION_HPP_
#define COMMON_VERSION_HPP_

#include <string_view>

namespace BuildInfo {
// 使用 string_view 避免静态对象的构造开销
constexpr std::string_view PROJECT_NAME = "Workout_Tracer";
constexpr std::string_view VERSION = "0.2.0";
constexpr std::string_view BUILD_DATE = __DATE__; // 编译器自动生成的日期
} // namespace BuildInfo

#endif // COMMON_VERSION_HPP_