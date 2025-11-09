#ifndef SQLEXECUTOR_H
#define SQLEXECUTOR_H

#include "interfaces/ISqlExecutor.h"

#include "interfaces/IDataBase.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <stdexcept>
#include <string>

#include "Debug_profiling.h"

class SqlExecutor : public ISqlExecutor {
  public:
    explicit SqlExecutor(IDataBase& database);
    bool execute(const QString& sql,
                 const QList<QVariant>& values,
                 QSqlQuery& outQuery) override;
    std::optional<long long> executeReturningId(const QString& sql, const QList<QVariant>& values,
                                                       QSqlQuery& outQuery) override;

  private:
    IDataBase& database_;
};

#endif // SQLEXECUTOR_H
