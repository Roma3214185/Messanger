#ifndef MOCKAUTORITIZER_H
#define MOCKAUTORITIZER_H

#include "interfaces/IAutoritizer.h"

class MockAutoritizer : public IAutoritizer {
 public:
  std::optional<long long> mock_user_id;
  std::string              last_token;
  int                      call_autoritize = 0;
  bool                     need_fail       = false;

  std::optional<long long> autoritize(const std::string& token) override {
    ++call_autoritize;
    last_token = token;
    if (need_fail) return std::nullopt;
    return mock_user_id;
  }
};

#endif  // MOCKAUTORITIZER_H
