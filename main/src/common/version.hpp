// src/common/version.hpp

#ifndef VERSION_HPP
#define VERSION_HPP

#include <string_view>

namespace BuildInfo {
    // 使用 string_view 避免静态对象的构造开销
    constexpr std::string_view PROJECT_NAME = "Workout_Tracer";
    constexpr std::string_view VERSION = "0.1.0";
    constexpr std::string_view BUILD_DATE = __DATE__; // 编译器自动生成的日期
}

#endif // VERSION_HPP