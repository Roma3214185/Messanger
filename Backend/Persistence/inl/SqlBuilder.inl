#pragma once

template <typename T>
QVariant toVariant(const Field& f, const T& entity) {
  std::any val = f.get(&entity);

  LOG_INFO("'{}' type: '{}' any type '{}'", f.name, f.type.name(),
           val.type().name());

  if (f.type == typeid(long long))
    return QVariant::fromValue(std::any_cast<long long>(val));
  if (f.type == typeid(std::string))
    return QString::fromStdString(std::any_cast<std::string>(val));
  if (f.type == typeid(QDateTime)) {
    QDateTime dt = std::any_cast<QDateTime>(val);

    if (!dt.isValid()) {
      LOG_WARN("in to variant was invalid datetimp");
      dt = QDateTime::currentDateTime();
    }

    return QVariant(dt.toSecsSinceEpoch());
  }
  return {};
}

template <typename T>
std::pair<QStringList, QStringList> buildInsertParts(
    const Meta& meta, const T& entity, QList<QVariant>& values) {
  QStringList cols, ph;
  for (const auto& f : meta.fields) {
    //if (std::string(f.name) == "id" &&  //TODO(roma) make another field id for this tables
    // //     std::string(meta.table_name) != "messages_status" &&
    // //     std::string(meta.table_name) != "chat_members")
    //continue;
    if (std::string(f.name) == "id" && toVariant<T>(f, entity) == 0) continue;

    cols << f.name;
    ph << "?";
    values << toVariant<T>(f, entity);
  }
  return {cols, ph};
}

template<typename T>
SqlStatement SqlBuilder<T>::buildInsert(const Meta& meta, const T& entity, bool need_to_return_id) {
  SqlStatement res;
  auto values =  QList<QVariant>{};
  auto [columns, placeholders] = buildInsertParts(meta, entity, values);
  res.values = values;

  QString row_sql = "INSERT OR REPLACE INTO %1 (%2) VALUES (%3)";
  if(need_to_return_id) row_sql += "RETURNING id";

  res.query = row_sql
              .arg(QString::fromStdString(meta.table_name))
              .arg(columns.join(", "))
              .arg(placeholders.join(", "));
  return res;
}
