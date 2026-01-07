#ifndef MOCKQUERY_H
#define MOCKQUERY_H

#include "interfaces/IQuery.h"

#if defined(__GNUC__) || defined(__clang__)
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

class MockQuery : public IQuery {
public:
  void bind(const QVariant &v) override { bindings.push_back(v); }

  bool exec() override { return !exec_should_fail; }

  bool next_should_fail = true;

  bool next() override { return !next_should_fail; }

  QVariant NOINLINE value(int i) const override { return mock_variant; }

  QVariant NOINLINE value(const std::string &field) const override {
    return mock_variant;
  }

  QString error() override {
    return mock_error;
  }

  QString mock_error = "mock_error";
  QVariant mock_variant = QVariant("4");
  bool exec_should_fail = false;
  std::vector<QVariant> bindings;
};

// todo: make different easier for tests fuucntion: valueString(), valueInt(),
// in real use retunr value().toString()

#endif // MOCKQUERY_H
