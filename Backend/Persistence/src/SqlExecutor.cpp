#include "SqlExecutor.h"

SqlExecutor::SqlExecutor(IDataBase& database)
    : database_(database) {}

bool SqlExecutor::execute(const QString& sql,
             const QList<QVariant>& values,
             QSqlQuery& outQuery) {
  PROFILE_SCOPE("[SqlExecutor] Execute");

  auto tread_db = database_.getThreadDatabase();
  if (!tread_db.isValid()) {
    LOG_ERROR("[SqlExecutor] Invalid database connection");
    throw std::runtime_error("Invalid database connection");
  }

  outQuery = QSqlQuery(tread_db);
  if (!outQuery.prepare(sql)) {
    LOG_ERROR("[SqlExecutor] Prepare failed: '{}'",
              outQuery.lastError().text().toStdString());
    return false;
  }

  for (int i = 0; i < values.size(); ++i)
    outQuery.bindValue(i, values[i]);

  LOG_INFO("[SqlExecutor] Executing SQL: {}", sql.toStdString());

  if (!outQuery.exec()) {
    LOG_ERROR("[SqlExecutor] Exec failed: '{}'",
              outQuery.lastError().text().toStdString());
    return false;
  }

  LOG_INFO("[SqlExecutor] Success: affected rows = {}",
           outQuery.numRowsAffected());

  return true;
}
