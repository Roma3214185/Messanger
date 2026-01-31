#ifndef SQLITEQUERY_H
#define SQLITEQUERY_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include "Debug_profiling.h"
#include "interfaces/IQuery.h"

class SQLiteQuery : public IQuery {
 public:
  explicit SQLiteQuery(const QSqlDatabase &db);

  bool prepare(const QString &sql);

  void bind(const QVariant &v) override;

  bool exec() override;

  bool next() override;

  QVariant value(int i) const override;

  QVariant value(const std::string &field) const override;

  QString error() override;

 private:
  QSqlQuery q_;
};

#endif  // SQLITEQUERY_H
