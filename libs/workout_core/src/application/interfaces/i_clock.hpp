#ifndef APPLICATION_INTERFACES_I_CLOCK_HPP_
#define APPLICATION_INTERFACES_I_CLOCK_HPP_

#include <string>

class IClock {
public:
  virtual ~IClock() = default;

  // Returns current UTC timestamp in ISO-8601 format.
  [[nodiscard]] virtual auto NowUtcIso8601() const -> std::string = 0;
};

#endif  // APPLICATION_INTERFACES_I_CLOCK_HPP_
