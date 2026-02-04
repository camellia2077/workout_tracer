// domain/models/workout_item.hpp
#ifndef DOMAIN_MODELS_WORKOUT_ITEM_HPP_
#define DOMAIN_MODELS_WORKOUT_ITEM_HPP_

#include <numeric>
#include <string>
#include <vector>

// 定义每一组的具体数据
struct SetData {
  int set_number_;          // 组号 (e.g., 1, 2, 3...)
  double weight_;           // 这组的重量
  int reps_;                // 这组的次数
  double volume_{0.0};      // volume = weight * reps
  std::string note_;        // set note

  [[nodiscard]] auto CalculateEpley() const -> double {
    if (reps_ <= 1) return weight_;
    return weight_ * (1.0 + static_cast<double>(reps_) / 30.0);
  }

  [[nodiscard]] auto CalculateBrzycki() const -> double {
    if (reps_ <= 1) return weight_;
    if (reps_ >= 37) return weight_ * 36.0; // Avoid division by zero/negative
    return weight_ * (36.0 / (37.0 - static_cast<double>(reps_)));
  }
};

struct ProjectData {
  std::string project_name_;   // 运动的名称
  std::string note_;           // project note
  std::string type_;           // 运动的类型,例如卧推是push
  std::vector<SetData> sets_;  // 包含所有组的向量
  double total_volume_{0.0};   // 这个项目的总容量
  int line_number_;            // 该项目在文件中的起始行号
};

struct DailyData {
  std::string date_;
  std::string note_;
  std::vector<ProjectData> projects_;
};

#endif // DOMAIN_MODELS_WORKOUT_ITEM_HPP_
