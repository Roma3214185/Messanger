#ifndef IQUERY_H
#define IQUERY_H

#include <QVariant>

struct IQuery {
  virtual ~IQuery() = default;
  virtual void bind(const QVariant &v) = 0;
  virtual bool exec() = 0;
  virtual bool next() = 0;
  virtual QString error() = 0;
  virtual QVariant value(int i) const = 0;
  virtual QVariant value(const std::string &field) const = 0;
};

#endif  // IQUERY_H
