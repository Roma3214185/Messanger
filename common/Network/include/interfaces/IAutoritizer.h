#ifndef IAUTORITIZER_H
#define IAUTORITIZER_H

#include <optional>

class IAutoritizer {
 public:
  virtual ~IAutoritizer() = default;
  virtual std::optional<long long> autoritize(const std::string &token) = 0;
};

#endif  // IAUTORITIZER_H
