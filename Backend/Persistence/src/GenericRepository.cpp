#include "Persistence/GenericRepository.h"

// QSqlQuery& GenericRepository::getPreparedQuery(const std::string& stmtKey, const QString& sql) {
//   QSqlDatabase threaddatabase_ = database_.getThreadDatabase();

//   auto it = stmt_cache_.find(stmtKey);
//   if (it == stmt_cache_.end()) {
//     QSqlQuery query(threaddatabase_);
//     query.prepare(sql);
//     auto [insertIt, _] = stmt_cache_.emplace(stmtKey, std::move(query));
//     return insertIt->second;
//   } else {
//     it->second.finish();
//     it->second.clear();
//     return it->second;
//   }
// }

GenericRepository::GenericRepository(ISqlExecutor& executor, ICacheService& cache, ThreadPool* pool)
    : executor_(executor), cache_(cache), pool_(pool) {}

void GenericRepository::clearCache() { } //cache_.clearCache(); }

std::vector<QList<QVariant>> GenericRepository::getValues() const {
  // std::vector<QList<QVariant>> allValues;
  // for (auto& entity : entities) {
  //   auto values = QList<QVariant>{};
  //   auto [columns, placeholders] = buildInsertParts(meta, entity, values);

  //   if (allColumns.empty()) {
  //     allColumns = QStringList();
  //     for (const auto& c : columns) allColumns << c;
  //   }

  //   allValues.push_back(values);
  // }
  // return allValues;
}
