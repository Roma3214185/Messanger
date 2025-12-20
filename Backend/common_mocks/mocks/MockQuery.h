#ifndef MOCKQUERY_H
#define MOCKQUERY_H

#include "interfaces/IQuery.h"

class MockQuery : public IQuery {
  public:
    void bind(const QVariant& v) override {
      bindings.push_back(v);
    }

    bool exec() override {
      return !exec_should_fail;
    }

    bool next_should_fail = true;

    bool next() override {
      return !next_should_fail;
    }

    QVariant value(int i) const override {
      return mock_variant;
      //todo: make different easier for tests fuucntion: valueString(), valueInt(), in real use retunr value().toString()
    }

    QVariant value(const std::string& field) const override {
      return mock_variant;
    }

    QVariant mock_variant = QVariant("4");
    bool exec_should_fail = false;
    std::vector<QVariant> bindings;
};

#endif  // MOCKQUERY_H
