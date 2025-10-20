#include "Query.h"

// template<typename T>
// T Query<T>::buildEntity(QSqlQuery& query, const Meta& meta) const {
//     T entity;
//     for (const auto& f : meta.fields) {
//         QVariant v = query.value(f.name);
//         if (!v.isValid()) continue;

//         std::any val;
//         if (f.type == typeid(long long)) val = v.toLongLong();
//         else if (f.type == typeid(std::string)) val = v.toString().toStdString();
//         else if (f.type == typeid(QDateTime)) val = v.toDateTime();
//         f.set(&entity, val);
//     }
//     return entity;
// }
