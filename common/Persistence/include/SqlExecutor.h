#ifndef SQLEXECUTOR_H
#define SQLEXECUTOR_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <stdexcept>
#include <string>

#include "Debug_profiling.h"
#include "interfaces/IDataBase.h"
#include "interfaces/ISqlExecutor.h"

class SqlExecutor : public ISqlExecutor {
 public:
  explicit SqlExecutor(IDataBase& database);
  bool execute(const QString& sql, QSqlQuery& outQuery, const QList<QVariant>& values = {}) override;
  //std::optional<long long> executeReturningId(const QString&         sql,
  //                                            QSqlQuery&             outQuery,
  //                                            const QList<QVariant>& values = {}) override;

 private:
  IDataBase& database_;
};

#endif  // SQLEXECUTOR_H
