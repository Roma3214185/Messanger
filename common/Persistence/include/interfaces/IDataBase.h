#ifndef IDATABASE_H
#define IDATABASE_H

#include <QString>

#include "interfaces/IQuery.h"

class IDataBase {
 public:
  virtual ~IDataBase() = default;

  virtual bool exec(const QString &sql) = 0;
  virtual std::unique_ptr<IQuery> prepare(const QString &sql) = 0;
  virtual std::unique_ptr<IQuery> prepare(const std::string &sql) = 0;
  virtual bool commit() = 0;
  virtual void rollback() = 0;
  virtual bool transaction() = 0;
};

#endif  // IDATABASE_H
