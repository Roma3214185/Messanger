#include "query/SQLiteQuery.h"

SQLiteQuery::SQLiteQuery(const QSqlDatabase &db) : q_(db) {}

bool SQLiteQuery::prepare(const QString &sql) {
  bool res = q_.prepare(sql);
  if (!res) {
    LOG_ERROR("[SQLiteQuery] Prepare failed for sql {}: {}", sql.toStdString(), q_.lastError().text().toStdString());
  }
  return res;
}

void SQLiteQuery::bind(const QVariant &v) { q_.addBindValue(v); }

bool SQLiteQuery::exec() {
  bool res = q_.exec();
  // LOG_INFO("Exec : {}", res);
  if (!res) LOG_INFO("Error {}", q_.lastError().text().toStdString());
  return res;
}

bool SQLiteQuery::next() { return q_.next(); }

QVariant SQLiteQuery::value(int i) const { return q_.value(i); }

QVariant SQLiteQuery::value(const std::string &field) const { return q_.value(field); }

QString SQLiteQuery::error() { return q_.lastError().text(); }
