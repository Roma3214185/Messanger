#ifndef GENERICREPOSIROTY_H
#define GENERICREPOSIROTY_H

#include <vector>
#include <functional>
#include "../RedisCashe/RedisCache.h"
#include <qthread.h>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QtSql/qsqlquery.h>
#include <iostream>
#include "../../DebugProfiling/Debug_profiling.h"
#include "Query.h"
#include "Meta.h"
#include <QDateTime>
#include "SQLiteDataBase.h"
#include "../MessageService/Headers/Message.h"
#include "ThreadPool.h"
#include "IEntityBuilder.h"

template <typename T>
using ResultList = std::vector<T>;
template <typename T>
using FutureResultList = std::future<std::vector<T>>;

class GenericRepository {
    IDataBase& db;
    RedisCache& cache = RedisCache::instance();
    ThreadPool* pool;
    std::unordered_map<std::string, QSqlQuery> stmtCache; // static tread_local (?)
public:
    explicit GenericRepository(IDataBase& db, ThreadPool* pool = nullptr) : db(db), pool(pool) {}

    template<typename T>
    void save(T& entity) {
        PROFILE_SCOPE("[repository] Save");
        auto meta = Reflection<T>::meta();
        LOG_INFO("Save in db: '{}'", meta.tableName);

        const Field* idField = meta.find("id");
        if (!idField) {
            LOG_ERROR("[save] Missing id field in meta");
            throw std::runtime_error("Missing 'id' field in meta");
        }

        auto id = std::any_cast<long long>(idField->get(&entity));
        LOG_INFO("[repository] [save] id is '{}'", id);
        bool isInsert = (id == 0);

        auto values = QList<QVariant>{};
        auto [columns, placeholders] = buildInsertParts(meta, entity, values);

        QSqlDatabase conn = db.getThreadDatabase();
        std::string stmtKey = meta.tableName + std::string(":save:") + std::to_string(columns.size());
        QString sql = QString("INSERT OR REPLACE INTO %1 (%2) VALUES (%3)")
                  .arg(QString::fromStdString(meta.tableName))
                  .arg(columns.join(", "))
                  .arg(placeholders.join(", "));

        auto& query = getPreparedQuery(stmtKey, sql);
        LOG_INFO("[repository] [save] values to insert size is '{}'", values.size());

        for (int i = 0; i < values.size(); ++i)
            query.bindValue(i, values[i]);

        if (!query.exec()){
            LOG_ERROR("[repository] Save failed: '{}'", query.lastError().text().toStdString());
            throw std::runtime_error(("Save failed: " + query.lastError().text()).toStdString());
        }

        LOG_INFO("[repository] Save successed");

        if (isInsert) {
            QVariant newId = query.lastInsertId();
            if (newId.isValid()) {
                LOG_INFO("id is valid: '{}'", newId.toLongLong());
                idField->set(&entity, newId.toLongLong());
            }else{
                LOG_ERROR("[repository] id isn't valid:");
            }
        }

        cache.remove(makeKey<T>(id));
        cache.incr(std::string("table_generation:") + meta.tableName);
    }

    template<typename T>
    void saveAsync(T& entity) {
        if(!pool){
            LOG_WARN("Pool isn't initialized");
            save(entity);
        }else{
            LOG_INFO("Start save async");
            pool->enqueue([this, entity]() { return this->save<T>(entity); });
        }
    }

    template<typename T>
    std::future<std::optional<T>> findOneAsync(long long id) {
        if(!pool){
            LOG_WARN("Pool isn't initialized");
            return std::async(std::launch::deferred, [this, id]() { return this->findOne<T>(id); });
        }else{
            LOG_INFO("Start save async");
            return pool->enqueue([this, id]() { return this->findOne<T>(id); });
        }
    }

    template<typename T>
    std::future<std::optional<T>> findOneWithOutCacheAsync(long long id) {
        if(!pool){
            LOG_WARN("Pool isn't initialized");
            return std::async(std::launch::async, [this, id]() {
                return this->findOneWithOutCache<T>(id);
            });
        } else {
            return pool->enqueue([this, id]() {
                return this->findOneWithOutCache<T>(id);
            });
        }
    }

    template<typename T>
    std::optional<T> findOne(long long id) {
        PROFILE_SCOPE("[repository] FindOne");
        const std::string key = makeKey<T>(id);
        if (auto cached = cache.get<T>(key)) {
            LOG_INFO("[repository] [CACHE HIT] key = '{}'", key);
            return cached;
        }
        LOG_INFO("[repository] cashe not hit key = '{}'", key);

        QSqlDatabase threadDb = db.getThreadDatabase();
        auto meta = Reflection<T>::meta();

        QString sql = QString("SELECT * FROM %1 WHERE id = ?").arg(meta.tableName);
        std::string stmtKey = std::string(meta.tableName) + ":findOne";

        auto& query = getPreparedQuery(stmtKey, sql);

        query.bindValue(0, id);

        if (!query.exec()) {
            LOG_ERROR("[repository] SQL error on '{}': {}", meta.tableName, query.lastError().text().toStdString());
            return std::nullopt;
        }

        if (!query.next()) {
            LOG_WARN("[repository] no rows found in '{}'", meta.tableName);
            return std::nullopt;
        }

        T entity = buildEntity<T>(query);
        cache.set(key, entity);
        return entity;
    }

    QSqlQuery& getPreparedQuery(const std::string& stmtKey, const QString& sql) {
        QSqlDatabase threadDb = db.getThreadDatabase();

        auto it = stmtCache.find(stmtKey);
        if (it == stmtCache.end()) {
            QSqlQuery query(threadDb);
            query.prepare(sql);
            auto [insertIt, _] = stmtCache.emplace(stmtKey, std::move(query));
            return insertIt->second;
        } else {
            it->second.clear();
            return it->second;
        }
    }


    template<typename T>
    std::optional<T> findOneWithOutCache(long long id) {
        const std::string key = makeKey<T>(id);
        QSqlDatabase threadDb = db.getThreadDatabase();
        auto meta = Reflection<T>::meta();

        std::string stmtKey = std::string(meta.tableName) + ":findOne";
        QString sql = QString("SELECT * FROM %1 WHERE id = ?").arg(meta.tableName);
        auto& query = getPreparedQuery(stmtKey, sql);

        query.bindValue(0, id);

        if (!query.exec()) {
            LOG_ERROR("[repository] SQL error on '{}': {}", meta.tableName, query.lastError().text().toStdString());
            return std::nullopt;
        }

        if (!query.next()) {
            LOG_WARN("[repository] no rows found in '{}'", meta.tableName);
            return std::nullopt;
        }

        return buildEntity<T>(query);
    }

    template<typename T>
    void deleteEntity(T& entity) {
        deleteById(entity.id);
    }

    template<typename T>
    void deleteById(long long id) {
        PROFILE_SCOPE("[repository] DeleteById");
        auto meta = Reflection<T>::meta();
        QString sql = QString("DELETE FROM %1 WHERE id = ?")
                          .arg(QString::fromStdString(meta.tableName));

        std::string stmKey = meta.tableName + std::string(":deleteById");
        auto& query = getPreparedQuery(stmKey, sql);

        query.bindValue(0, id);

        if (!query.exec()){
            LOG_ERROR("[repository] Delete failed: '{}'", query.lastError().text().toStdString());
            throw std::runtime_error("Delete failed: " + query.lastError().text().toStdString());
        }
        cache.incr("table_generation:" + meta.tableName);
        cache.remove(makeKey<T>(id));
    }

    template<typename T>
    std::vector<T> findBy(const std::string& field, const std::string& value) {
        return findBy<T>(field, QVariant(QString::fromStdString(value)));
    }

    template<typename T>
    std::vector<T> findBy(const std::string& field, const QVariant& value) {
        return this->query<T>().filter(field, value).execute();
    }

    template<typename T>
    bool exists(long long id) {
        PROFILE_SCOPE("Exist");
        const std::string key = makeKey<T>(id);
        if (cache.exists(key)) {
            LOG_INFO("[repository] CACHE HIT in exist true, key '{}'", key);
            return true;
        }

        auto meta = Reflection<T>::meta();
        LOG_INFO("[repository] CACHE not HIT in db '{}', key '{}'", meta.tableName, key);
        QString sql = QString("SELECT COUNT(1) FROM %1 WHERE id = ?")
                          .arg(QString::fromStdString(meta.tableName));
        std::string stmKey = meta.tableName + std::string(":exists");
        auto& query = getPreparedQuery(stmKey, sql);
        query.bindValue(0, id);

        if (!query.exec() || !query.next()) {
            LOG_ERROR("Id '{}' doen't exist", id);
            return false;
        }
        bool found = query.value(0).toLongLong() > 0;
        if (found) cache.set(key, true, std::chrono::minutes(5));
        return found;
    }

    template<typename T>
    void truncate() {
        LOG_INFO("[repository] Truncate db '{}'");
        auto meta = Reflection<T>::meta();
        QString sql = QString("DELETE FROM %1").arg(QString::fromStdString(meta.tableName));
        std::string stmKey = meta.tableName + std::string(":truncate");
        auto& query = getPreparedQuery(stmKey, sql);
        if (!query.exec())
            throw std::runtime_error(("Truncate failed: " + query.lastError().text()).toStdString());
        cache.clearPrefix(meta.tableName + ":");
    }

    template<typename T>
    Query<T> query() {
        return Query<T>(this->db);
    }

    template<typename T>
    T buildEntity(QSqlQuery& query, BuilderType type = BuilderType::Fast) const {
        auto builder = makeBuilder<T>(type);
        return builder->build(query);
    }

private:

    template<typename T>
    std::string makeKey(const T& entity) const{
        return makeKey(entity.id);
    }

    template<typename T>
    std::string makeKey(long long id) const {
        return "entity_cache:" + std::string(Reflection<T>::meta().tableName) + ":" + std::to_string(id);
    }

    template<typename T>
    long long getId(const T& obj) const {
        auto meta = Reflection<T>::meta();
        if (auto f = meta.find("id"))
            return std::any_cast<long long>(f->get(&obj));
        return 0;
    }

    template<typename T>
    QStringList buildUpdateParts(const Meta& meta, const T& entity, QList<QVariant>& values) {
        QStringList sets;
        for (const auto& f : meta.fields) {
            if (std::string(f.name) == "id" && std::string(meta.tableName) != "messages_status") continue;
            sets << QString("%1 = ?").arg(f.name);
            values << toVariant(f, entity);
        }
        return sets;
    }

    template<typename T>
    std::pair<QStringList, QStringList> buildInsertParts(const Meta& meta, const T& entity, QList<QVariant>& values) {
        QStringList cols, ph;
        for (const auto& f : meta.fields) {
            if (std::string(f.name) == "id" && std::string(meta.tableName) != "messages_status") continue;
            cols << f.name;
            ph << "?";
            values << toVariant<T>(f, entity);
        }
        return {cols, ph};
    }

    template<typename T>
    QVariant toVariant(const Field& f, const T& entity) const {
        std::any val = f.get(&entity);

        LOG_INFO("'{}' type: '{}' any type '{}'", f.name, f.type.name(), val.type().name());

        if (f.type == typeid(long long)) return QVariant::fromValue(std::any_cast<long long>(val));
        if (f.type == typeid(std::string)) return QString::fromStdString(std::any_cast<std::string>(val));
        if (f.type == typeid(QDateTime)) {
            QDateTime dt = std::any_cast<QDateTime>(val);

            if (!dt.isValid()){
                LOG_WARN("in to variant was invalid datetimp");
                dt = QDateTime::currentDateTime();
            }

            return QVariant(dt.toSecsSinceEpoch());
        }
        return {};
    }
};

#endif // GENERICREPOSIROTY_H
