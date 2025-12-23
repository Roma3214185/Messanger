#ifndef IDFIELD_H
#define IDFIELD_H

#include <optional>

#include "Debug_profiling.h"

struct IdField {
    static std::optional<IdField> tryCreate(long long v) {
      if (v <= 0) return std::nullopt;
      return IdField(v);
    }

    long long operator()() const { return value_; }

  private:
    explicit IdField(long long v) {
      if(v <= 0) {  //todo: add [[unlikely]]
        LOG_ERROR("IdField can't be <= 0");
        throw std::runtime_error("Invalid id");
      }

      value_(v);
    }
    long long value_;
};


#endif // IDFIELD_H
