// common/c_json_helper.hpp

#ifndef COMMON_C_JSON_HELPER_HPP_
#define COMMON_C_JSON_HELPER_HPP_

#include <cjson/cJSON.h>
#include <memory>
#include <string>

// 自定义删除器，调用 cJSON_Delete
struct CJsonDeleter {
  void operator()(cJSON* ptr) const {
    if (ptr != nullptr) {
      cJSON_Delete(ptr);
    }
  }
};

// 定义智能指针类型，自动管理 cJSON* 的生命周期
using CJsonPtr = std::unique_ptr<cJSON, CJsonDeleter>;

/**
 * @brief 辅助函数：将 cJSON* 转换为智能指针
 * @param ptr 原始 cJSON 指针
 * @return 包装后的智能指针
 */
inline auto MakeCJson(cJSON* ptr) -> CJsonPtr {
  return CJsonPtr(ptr);
}

#endif // COMMON_C_JSON_HELPER_HPP_