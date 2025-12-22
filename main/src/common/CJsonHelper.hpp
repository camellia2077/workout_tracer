// common/CJsonHelper.hpp
#ifndef CJSON_HELPER_HPP
#define CJSON_HELPER_HPP

#include <cjson/cJSON.h>
#include <memory>
#include <string>

// 自定义删除器，调用 cJSON_Delete
struct CJsonDeleter {
    void operator()(cJSON* ptr) const {
        if (ptr) cJSON_Delete(ptr);
    }
};

// 定义智能指针类型，自动管理 cJSON* 的生命周期
using CJsonPtr = std::unique_ptr<cJSON, CJsonDeleter>;

// 辅助函数：将 cJSON* 转换为智能指针
inline CJsonPtr make_cjson(cJSON* ptr) {
    return CJsonPtr(ptr);
}

#endif // CJSON_HELPER_HPP