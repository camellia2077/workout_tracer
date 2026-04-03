#ifndef DOMAIN_SERVICES_WEIGHT_UNIT_SERVICE_HPP_
#define DOMAIN_SERVICES_WEIGHT_UNIT_SERVICE_HPP_

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace WeightUnitService {

inline constexpr std::string_view kKgUnit = "kg";
inline constexpr std::string_view kLbUnit = "lb";
inline constexpr std::string_view kDisplayOriginal = "original";
inline constexpr double kLbToKgFactor = 0.45359237;

struct DisplayWeight {
  double value_ = 0.0;
  std::string unit_ = std::string(kKgUnit);
};

inline auto ToLower(std::string_view value) -> std::string {
  std::string lowered(value);
  std::ranges::transform(lowered, lowered.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return lowered;
}

inline auto NormalizeOriginalUnit(std::string_view unit)
    -> std::optional<std::string> {
  const std::string lowered = ToLower(unit);
  if (lowered.empty() || lowered == kKgUnit) {
    return std::string(kKgUnit);
  }
  if (lowered == "l" || lowered == "lb" || lowered == "lbs") {
    return std::string(kLbUnit);
  }
  return std::nullopt;
}

inline auto NormalizeDisplayUnit(std::string_view unit)
    -> std::optional<std::string> {
  const std::string lowered = ToLower(unit);
  if (lowered.empty() || lowered == kDisplayOriginal) {
    return std::string(kDisplayOriginal);
  }
  if (lowered == kKgUnit) {
    return std::string(kKgUnit);
  }
  if (lowered == kLbUnit) {
    return std::string(kLbUnit);
  }
  return std::nullopt;
}

inline auto ConvertToKg(double value, std::string_view unit) -> double {
  const std::string normalized =
      NormalizeOriginalUnit(unit).value_or(std::string(kKgUnit));
  if (normalized == kLbUnit) {
    return value * kLbToKgFactor;
  }
  return value;
}

inline auto ConvertFromKg(double value_kg, std::string_view unit) -> double {
  const std::string normalized =
      NormalizeOriginalUnit(unit).value_or(std::string(kKgUnit));
  if (normalized == kLbUnit) {
    return value_kg / kLbToKgFactor;
  }
  return value_kg;
}

inline auto ResolveDetailDisplayUnit(std::string_view requested_display_unit,
                                     std::string_view original_unit)
    -> std::string {
  const std::string normalized_display =
      NormalizeDisplayUnit(requested_display_unit)
          .value_or(std::string(kDisplayOriginal));
  if (normalized_display == kDisplayOriginal) {
    return NormalizeOriginalUnit(original_unit).value_or(std::string(kKgUnit));
  }
  return normalized_display;
}

inline auto ResolveAggregateDisplayUnit(std::string_view requested_display_unit,
                                        std::string_view common_original_unit)
    -> std::string {
  const std::string normalized_display =
      NormalizeDisplayUnit(requested_display_unit)
          .value_or(std::string(kDisplayOriginal));
  if (normalized_display != kDisplayOriginal) {
    return normalized_display;
  }
  const auto normalized_original = NormalizeOriginalUnit(common_original_unit);
  if (normalized_original.has_value()) {
    return normalized_original.value();
  }
  return std::string(kKgUnit);
}

inline auto ResolveDetailWeight(double weight_kg, std::string_view original_unit,
                                double original_weight_value,
                                std::string_view requested_display_unit)
    -> DisplayWeight {
  const std::string unit =
      ResolveDetailDisplayUnit(requested_display_unit, original_unit);
  if (NormalizeDisplayUnit(requested_display_unit) ==
      std::optional<std::string>(std::string(kDisplayOriginal))) {
    return {.value_ = original_weight_value, .unit_ = unit};
  }
  return {.value_ = ConvertFromKg(weight_kg, unit), .unit_ = unit};
}

inline auto ResolveAggregateWeight(double weight_kg,
                                   std::string_view requested_display_unit,
                                   std::string_view common_original_unit)
    -> DisplayWeight {
  const std::string unit = ResolveAggregateDisplayUnit(requested_display_unit,
                                                       common_original_unit);
  return {.value_ = ConvertFromKg(weight_kg, unit), .unit_ = unit};
}

inline auto FormatWeightValue(double value, int precision = 3) -> std::string {
  constexpr double kZeroEpsilon = 0.0000005;
  if (std::abs(value) < kZeroEpsilon) {
    value = 0.0;
  }

  std::ostringstream stream;
  stream << std::fixed << std::setprecision(precision) << value;
  std::string text = stream.str();
  if (text.find('.') != std::string::npos) {
    while (!text.empty() && text.back() == '0') {
      text.pop_back();
    }
    if (!text.empty() && text.back() == '.') {
      text.pop_back();
    }
  }
  if (text == "-0") {
    return "0";
  }
  return text;
}

inline auto FormatMetricValue(double value, int precision = 1) -> std::string {
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(precision) << value;
  return stream.str();
}

inline auto FormatWeightWithUnit(double value, std::string_view unit,
                                 int precision = 3) -> std::string {
  return FormatWeightValue(value, precision) + std::string(unit);
}

}  // namespace WeightUnitService

#endif  // DOMAIN_SERVICES_WEIGHT_UNIT_SERVICE_HPP_
