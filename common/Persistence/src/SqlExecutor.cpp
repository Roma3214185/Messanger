#include "SqlExecutor.h"

SqlExecutor::SqlExecutor(IDataBase& database) : database_(database) {}

std::unique_ptr<IQuery> SqlExecutor::execute(const QString& sql, const QList<QVariant>& values) {
  PROFILE_SCOPE("[SqlExecutor] Execute");

  // auto tread_db = database_.getThreadDatabase();
  // if (!tread_db.isValid()) {
  //   LOG_ERROR("[SqlExecutor] Invalid database connection");
  //   throw std::runtime_error("Invalid database connection");
  // }

  auto outQuery = database_.prepare(sql);
  if(!outQuery) return nullptr;
  // if (!outQuery.prepare(sql)) {
  //   LOG_ERROR("[SqlExecutor] Prepare failed: '{}'", outQuery.lastError().text().toStdString());
  //   return false;
  // }

  for (int i = 0; i < values.size(); ++i) outQuery->bind(values[i]);

  //LOG_INFO("[SqlExecutor] Executing SQL: {}", sql.toStdString());

  if (!outQuery->exec()) {
    LOG_ERROR("[SqlExecutor] Exec failed: '{}'", sql.toStdString());
    return nullptr;
  }

  LOG_INFO("[SqlExecutor] Exec succeed: '{}'", sql.toStdString());
  //LOG_INFO("[SqlExecutor] Success: affected rows = {}", outQuery.numRowsAffected());

  return outQuery;
}

// std::optional<long long> SqlExecutor::executeReturningId(const QString&         sql,
//                                                          QSqlQuery&             outQuery,
//                                                          const QList<QVariant>& values) {
//   if (!execute(sql, outQuery, values)) return std::nullopt;

//   if (!outQuery.next()) {
//     LOG_WARN("No row returned for SQL returning ID: {}", sql.toStdString());
//     return std::nullopt;
//   }

//   auto returned_id = outQuery.value(0).toLongLong();
//   LOG_INFO("Returned id fromt SqlExecuter {}", returned_id);
//   return returned_id;
// }
