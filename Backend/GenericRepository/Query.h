#ifndef QUERY_H
#define QUERY_H
#include <QtSql/QSqlDatabase>
#include "GenericReposiroty.h"
#include "../GenericRepository/GenericReposiroty.h"
#include "Meta.h"

struct Meta;

template<typename T>
struct Reflection;

template<typename T>
class Query {
public:
    Query(QSqlDatabase db)
        : db(std::move(db))
    {
        tableName = Reflection<T>::meta().tableName;
        involvedTables.push_back(tableName);
    }


    Query& filter(const std::string& field, const QVariant& value) {
        filters.push_back(QString("%1 = ?").arg(QString::fromStdString(field)));
        values.push_back(value);
        return *this;
    }

    Query& filter(const std::string& field, const std::string& op, const QVariant& value) {
        filters.push_back(QString("%1 %2 ?")
                              .arg(QString::fromStdString(field))
                              .arg(QString::fromStdString(op)));
        values.push_back(value);
        return *this;
    }

    Query& orderBy(const std::string& field, const std::string& direction = "ASC") {
        order = QString("ORDER BY %1 %2")
        .arg(QString::fromStdString(field))
            .arg(QString::fromStdString(direction));
        return *this;
    }

    Query& limit(int n) {
        limitClause = QString("LIMIT %1").arg(n);
        return *this;
    }

    std::vector<T> execute() const {
        QString sql = buildSelectQuery();
        auto generations = getGenerations();
        std::size_t generationHash = hashGenerations(generations);
        std::size_t paramsHash = hashParams(values);

        std::string cacheKey = createCacheKey(sql, generationHash, paramsHash);

        if (auto cached = cache.get<std::vector<T>>(cacheKey)) {
            LOG_INFO("[QueryCache] HIT for key '{}'", cacheKey);
            return *cached;
        }

        LOG_INFO("[QueryCache] NOT HITTED for key '{}'", cacheKey);

        QSqlQuery query(db);
        query.prepare(sql);
        for (int i = 0; i < values.size(); ++i)
            query.bindValue(i, values[i]);

        if (!query.exec()) {
            LOG_ERROR("Query error: {}", query.lastError().text().toStdString());
            return {};
        }

        auto results = std::vector<T>{};
        auto meta = Reflection<T>::meta();

        while (query.next()){
            T entity = buildEntity(query, meta);
            results.push_back(entity);
            saveEntityInCache(entity);
        }

        LOG_INFO("Result size is '{}' is setted in cashe for key '{}'", results.size(), cacheKey);
        cache.set(cacheKey, results, std::chrono::hours(24));
        return results;
    }

    bool deleteAll() const {
        QString sql = QString("DELETE FROM %1").arg(QString::fromStdString(tableName));
        if (!filters.empty())
            sql += " WHERE " + filters.join(" AND ");

        QSqlQuery query(db);
        query.prepare(sql);
        for (int i = 0; i < values.size(); ++i)
            query.bindValue(i, values[i]);

        if (!query.exec()) {
            LOG_ERROR("Delete query failed: {}", query.lastError().text().toStdString());
            return false;
        }

        LOG_INFO("Deleted {} rows from {}", query.numRowsAffected(), tableName);
        return true;
    }

private:

    void saveEntityInCache(const T& entity, std::chrono::hours ttl = std::chrono::hours(24)) const{
        std::string entityKey = buildEntityKey(entity);
        cache.set(entityKey, entity, ttl);
    }

    T buildEntity(QSqlQuery& query, const Meta& meta) const{
        T entity;
        for (const auto& f : meta.fields) {
            QVariant v = query.value(f.name);
            if (!v.isValid()) continue;

            std::any val;
            if (f.type == typeid(long long)) val = v.toLongLong();
            else if (f.type == typeid(std::string)) val = v.toString().toStdString();
            else if (f.type == typeid(QDateTime)) val = v.toDateTime();
            f.set(&entity, val);
        }
        return entity;
    }

    int getEntityId(T entity) const {
        return entity.id;
    }

    QString buildSelectQuery() const{
        QString sql = QString("SELECT * FROM %1").arg(QString::fromStdString(tableName));
        if (!filters.empty())
            sql += " WHERE " + filters.join(" AND ");
        if (!order.isEmpty())
            sql += " " + order;
        if (!limitClause.isEmpty())
            sql += " " + limitClause;
        return sql;
    }

    auto getGenerations() const{
        std::unordered_map<std::string, long long> generations;
         for (const auto& table : involvedTables) {
            std::string key = "table_generation:" + table;
            auto val = cache.get<std::string>(key);
            generations[table] = val ? std::stoll(*val) : 0;
        }
        return generations;
    }


    std::size_t hashGenerations(const std::unordered_map<std::string, long long>& generations) const{
        std::size_t generationHash = 0;
        for (const auto& [table, gen] : generations)
            generationHash ^= std::hash<std::string>{}(table + std::to_string(gen));
        return generationHash;
    }

    std::size_t hashParams(QVector<QVariant>) const{
        std::size_t paramsHash = 0;
        for (const auto& v : values)
            paramsHash ^= std::hash<std::string>{}(v.toString().toStdString());
        return paramsHash;
    }

    std::string buildEntityKey(const T& entity) const{
        return "entity_cache:" + tableName + ":" + std::to_string(getEntityId(entity));
    }

    std::string createCacheKey(QString sql, int generationHash, int paramsHash) const{
         std::string cacheKey = "query_cache:" + sql.toStdString()
                           + ":gen=" + std::to_string(generationHash)
                           + ":params=" + std::to_string(paramsHash);
        return cacheKey;
    }

private:
    RedisCache& cache = RedisCache::instance();
    std::vector<std::string> involvedTables;
    QSqlDatabase db;
    QStringList filters;
    QVector<QVariant> values;
    QString order;
    QString limitClause;
    std::string tableName;
};

#endif // QUERY_H
