#ifndef SQLITEQUERY_H
#define SQLITEQUERY_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "interfaces/IQuery.h"
#include "Debug_profiling.h"

class SQLiteQuery : public IQuery {
  public:
    explicit SQLiteQuery(QSqlDatabase db) : q_(db) {}

    bool prepare(const QString& sql) {
      bool res = q_.prepare(sql);
      if(!res) LOG_ERROR("[SQLiteQuery] Prepare failed for sql {}: {}", sql.toStdString(), q_.lastError().text().toStdString());
      return res;
    }

    void bind(const QVariant& v) override {
      q_.addBindValue(v);
    }

    bool exec() override {
      bool res = q_.exec();
      //LOG_INFO("Exec : {}", res);
      if(!res) LOG_INFO("Error {}", q_.lastError().text().toStdString());
      return res;
    }

    bool next() override {
      return q_.next();
    }

    QVariant value(int i) const override {
      return q_.value(i);
    }

    QVariant value(const std::string& field) const override {
      return q_.value(field);
    }

    std::string error() {
      return q_.lastError().text().toStdString();
    }

  private:
    QSqlQuery q_;
};


#endif // SQLITEQUERY_H
