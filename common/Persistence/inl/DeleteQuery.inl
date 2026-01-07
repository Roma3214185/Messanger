template <EntityJson T>
DeleteQuery<T>::DeleteQuery(ISqlExecutor* executor, ICacheService& cache)
    : IBaseQuery<T>(executor), cache_(cache) { }

template <EntityJson T>
QueryResult<T> DeleteQuery<T>::execute() const {
  QString sql = buildQuery();

  auto execute_results = this->executor_->execute(sql, this->values_);
  if(!execute_results.query) {
    LOG_ERROR("query {} failed, reason - {}", sql.toStdString(), execute_results.error);
    return DeleteResult<T>{ .success = false };
  }

  LOG_INFO("query {} succeed", sql.toStdString());
  for(const auto& table_name : this->involved_tables_) {
    cache_.incr(std::string("table_generation:") + table_name.toStdString());
  }

  return DeleteResult<T>{ .success = true };
}

template <EntityJson T>
QString DeleteQuery<T>::buildQuery() const {
  QString sql =
      QString("DELETE * FROM %1").arg(this->table_name_);
  sql += this->join_clause_;
  if (!this->filters_.empty()) sql += " WHERE " + this->filters_.join(" AND ");
  if (!this->limit_clause_.isEmpty()) sql += " " + this->limit_clause_;
  return sql;
}

