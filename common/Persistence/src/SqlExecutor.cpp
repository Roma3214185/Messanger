#include "SqlExecutor.h"

SqlExecutor::SqlExecutor(IDataBase &database) : database_(database) {}

std::unique_ptr<IQuery> SqlExecutor::execute(const QString &sql,
                                             const QList<QVariant> &values) {
  PROFILE_SCOPE("[SqlExecutor] Execute");

  auto outQuery = database_.prepare(sql);
  if (!outQuery) {
    LOG_ERROR("Error prepare outQuery");
    return nullptr;
  }

  for (int i = 0; i < values.size(); ++i)
    outQuery->bind(values[i]);

  LOG_INFO("[SqlExecutor] Executing SQL: {}", sql.toStdString());

  if (!outQuery->exec()) {
    LOG_ERROR("[SqlExecutor] Exec failed: '{}'", sql.toStdString());
    return nullptr;
  }

  LOG_INFO("[SqlExecutor] Exec succeed: '{}'", sql.toStdString());
  return outQuery;
}

// std::optional<long long> SqlExecutor::executeReturningId(const QString& sql,
//                                                          QSqlQuery& outQuery,
//                                                          const
//                                                          QList<QVariant>&
//                                                          values) {
//   if (!execute(sql, outQuery, values)) return std::nullopt;

//   if (!outQuery.next()) {
//     LOG_WARN("No row returned for SQL returning ID: {}", sql.toStdString());
//     return std::nullopt;
//   }

//   auto returned_id = outQuery.value(0).toLongLong();
//   LOG_INFO("Returned id fromt SqlExecuter {}", returned_id);
//   return returned_id;
// }
