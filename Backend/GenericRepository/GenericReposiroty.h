#ifndef GENERICREPOSIROTY_H
#define GENERICREPOSIROTY_H

#include <vector>
#include <functional>
#include "../RedisCashe/RedisCache.h"
#include <qthread.h>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QDateTime>
#include <QDebug>
#include <QtSql/qsqlquery.h>
#include <iostream>
#include "../../DebugProfiling/Debug_profiling.h"

class IDataBase {
public:
    virtual QSqlDatabase getThreadDatabase() = 0;
    virtual ~IDataBase() = default;
};

class SQLiteDatabase : public IDataBase {
public:
    explicit SQLiteDatabase(const QString& dbPath = "chat_db.sqlite")
        : dbPath(dbPath) {
        initializeSchema();
    }

    QSqlDatabase getThreadDatabase() override {
        const QString connName = QString("connection_%1").arg((quintptr)QThread::currentThreadId());
        QSqlDatabase db;
        if (QSqlDatabase::contains(connName))
            db = QSqlDatabase::database(connName);
        else {
            db = QSqlDatabase::addDatabase("QSQLITE", connName);
            db.setDatabaseName(dbPath);
        }
        if (!db.isOpen() && !db.open()){
            LOG_ERROR("Cann't open database");
            throw std::runtime_error("Cannot open database");
        }

        return db;
    }

    void intializeUserDb(){
        auto db = getThreadDatabase();
        QSqlQuery query(db);
        // if (!query.exec("DROP TABLE IF EXISTS users")) {
        //     qWarning() << "Failed to drop users table:" << query.lastError().text();
        // }

        if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                email TEXT UNIQUE NOT NULL,
                tag TEXT NOT NULL,
                username TEXT NOT NULL
            );
        )")) {
            spdlog::warn("Failed to create users table:", query.lastError().text().toStdString());
            return;
        }
        LOG_INFO("Messages_status is initialized");
    }

    void intializeMessageDb(){
        auto db = getThreadDatabase();
        QSqlQuery query(db); //SELECT datetime(timestamp, 'unixepoch') FROM events;
        // if (!query.exec("DROP TABLE IF EXISTS messages")) {
        //     qWarning() << "Failed to drop users table:" << query.lastError().text();
        // }

        if(!query.exec(R"(
        CREATE TABLE IF NOT EXISTS messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            chat_id INTEGER,
            sender_id INTEGER,
            text TEXT,
            timestamp INTEGER NOT NULL,
            FOREIGN KEY(chat_id) REFERENCES chats(id),
            FOREIGN KEY(sender_id) REFERENCES users(id)
            )
        )")){
            LOG_ERROR("Messages status isn't initialized: '{}'", query.lastError().text().toStdString());
            return;
        }

        query.exec(R"(CREATE INDEX IF NOT EXISTS idx_messages_chatid_time ON messages(chat_id, timestamp))");
        query.exec(R"(CREATE INDEX IF NOT EXISTS idx_messages_sender ON messages(sender_id))");
        LOG_INFO("Messages is initialized");
    }

    void intializeStatusDb(){
        auto db = getThreadDatabase();
        QSqlQuery query(db);

        // if (!query.exec("DROP TABLE IF EXISTS messages_status")) {
        //     qWarning() << "Failed to drop users table:" << query.lastError().text();
        // }

        if(!query.exec(R"(
            CREATE TABLE IF NOT EXISTS messages_status (
                id INTEGER,
                receiver_id INTEGER,
                is_read BOOLEAN,
                read_at INTEGER,
                FOREIGN KEY(receiver_id) REFERENCES users(id),
                PRIMARY KEY(id, receiver_id)
            );
        )")){
            LOG_ERROR("Messages_status status isn't initialized: '{}'", query.lastError().text().toStdString());
        }
        LOG_INFO("Messages_status is initialized");
    }

    void initializeChatsDb(){
        auto db = getThreadDatabase();
        QSqlQuery query(db);

        if(!query.exec(R"(
            query.exec(R"(
             CREATE TABLE IF NOT EXISTS chats (
                 id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                 is_group BOOLEAN NOT NULL,
                 created_at DATETIME DEFAULT CURRENT_TIMESTAMP
             );
        )")){
            LOG_ERROR("Messages_status status isn't initialized: '{}'", query.lastError().text().toStdString());
        }
        LOG_INFO("Messages_status is initialized");

    }

    void initializeSchema() {
        intializeUserDb();
        intializeMessageDb();
        intializeStatusDb();
        //initializeChatsDb();
    }

private:
    QString dbPath;
};

struct Field{
    const char* name;
    const std::type_info& type;
    std::function<std::any(const void*)> get;
    std::function<void(void*, const std::any&)> set;
};

struct Meta {
    const char* name;
    const char* tableName;
    std::vector<Field> fields;

    const Field* find(const std::string& n) const {
        for (const auto& f : fields)
            if (n == f.name) return &f;
        return nullptr;
    }
};

template <class T, class M>
Field make_field(const char* name, M T::* member) {
    return Field{
        .name = name,
        .type = typeid(M),
        .get = [member](const void* obj) -> std::any {
            const T* element = static_cast<const T*>(obj);
            return element->*member;
        },
        .set = [member](void* obj, const std::any& val) {
            T* element = static_cast<T*>(obj);

            if constexpr (std::is_same_v<M, QDateTime>){
                return element->*member; // just return QDateTime
            } else {
                if (!val.has_value()) return;
                if constexpr (std::is_same_v<M, std::string>) {
                    if (val.type() == typeid(const char*))
                        (element->*member) = std::string(std::any_cast<const char*>(val));
                    else
                        (element->*member) = std::any_cast<M>(val);
                } else {
                    (element->*member) = std::any_cast<M>(val);
                }
            }
        }
    };
}


template<typename T>
struct Reflection {
    static Meta meta();
};


class GenericRepository {
    IDataBase& db;
    RedisCache& cache = RedisCache::instance();

public:
    explicit GenericRepository(IDataBase& db) : db(db) {}

    template<typename T>
    void save(T& entity) {
        PROFILE_SCOPE("[repository] Save");
        QSqlDatabase conn = db.getThreadDatabase();
        QSqlQuery query(conn);
        auto meta = Reflection<T>::meta();
        LOG_INFO("Save in db: '{}'", meta.tableName);

        const Field* idField = meta.find("id");
        if (!idField) {
            LOG_ERROR("[save] Missing id field in meta");
            throw std::runtime_error("Missing 'id' field in meta");
        }

        long long id = std::any_cast<long long>(idField->get(&entity));
        LOG_INFO("[repository] [save] id is '{}'", id);
        bool isInsert = (id == 0);

        QString sql;
        QList<QVariant> values;

        if (isInsert) {
            auto [columns, placeholders] = buildInsertParts(meta, entity, values);
            std::cerr << "builded insert parts : " << id << std::endl;
            sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                      .arg(QString::fromStdString(meta.tableName))
                      .arg(columns.join(", "))
                      .arg(placeholders.join(", "));

        } else {
            QStringList sets = buildUpdateParts(meta, entity, values);
            sql = QString("UPDATE %1 SET %2 WHERE id = ?")
                      .arg(QString::fromStdString(meta.tableName))
                      .arg(sets.join(", "));
            values << id; // why???
        }
        query.prepare(sql);
        LOG_INFO("[[repository] [save] values size is '{}'", values.size());

        for (int i = 0; i < values.size(); ++i)
            query.bindValue(i, values[i]);

        if (!query.exec()){
            LOG_ERROR("[repository] Save failde: '{}'", query.lastError().text().toStdString());
            throw std::runtime_error(("Save failed: " + query.lastError().text()).toStdString());
        }
        LOG_INFO("[repository] Save successed");

        if (isInsert) {
            QVariant newId = query.lastInsertId();
            if (newId.isValid()) {
                LOG_INFO("id is valid: '{}'", newId.toLongLong() + 1);
                idField->set(&entity, newId.toLongLong() + 1);
            }else{
                LOG_ERROR("[repository] id isn't valid:");
            }
            cache.set(makeKey<T>(getId(entity)), entity, std::chrono::hours(24));
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
        QSqlQuery query(db.getThreadDatabase());
        auto meta = Reflection<T>::meta();
        LOG_INFO("Database '{}'", meta.tableName);
        query.prepare(QString("SELECT * FROM %1 WHERE id = ?")
                          .arg(QString::fromStdString(meta.tableName)));
        query.bindValue(0, id);

        if (!query.exec() || !query.next()) {
            LOG_ERROR("[repository] error select * from '{}', error: '{}'", meta.tableName, query.lastError().text().toStdString());
            return std::nullopt;
        }

        T entity = buildEntity<T>(query, meta);
        cache.set(key, entity);
        return entity;
    }

    template<typename T>
    void deleteById(long long id) {
        PROFILE_SCOPE("[repository] DeleteById");
        auto meta = Reflection<T>::meta();
        LOG_INFO("[repository] Databse: '{}'", meta.tableName);
        QSqlQuery query(db.getThreadDatabase());
        query.prepare(QString("DELETE FROM %1 WHERE id = ?")
                          .arg(QString::fromStdString(meta.tableName)));
        query.bindValue(0, id);

        if (!query.exec()){
            LOG_ERROR("[repository] Delete failed: '{}'", query.lastError().text().toStdString());
            throw std::runtime_error("Delete failed: " + query.lastError().text().toStdString());
        }

        cache.remove(makeKey<T>(id));
    }

    template<typename T>
    std::vector<T> findBy(const std::string& field, const std::string& value) {
        return findBy<T>(field, QVariant(QString::fromStdString(value)));
    }

    template<typename T>
    std::vector<T> findBy(const std::string& field, const QVariant& value) {
        PROFILE_SCOPE("[repository] FindBy");
        std::vector<T> results;
        auto meta = Reflection<T>::meta();

        if (!meta.find(field)){
            LOG_ERROR("[repository] Invalid field '{}'", field);
            throw std::invalid_argument("Invalid field: " + field);
        }

        QSqlQuery query(db.getThreadDatabase());
        query.prepare(QString("SELECT * FROM %1 WHERE %2 = ?")
                          .arg(QString::fromStdString(meta.tableName))
                          .arg(QString::fromStdString(field)));
        query.bindValue(0, value);

        if (!query.exec()) {
            LOG_ERROR("[repository] Error in findBy in table '{}', error:", meta.tableName, query.lastError().text().toStdString());
            return results;
        }
        while (query.next()) results.push_back(buildEntity<T>(query, meta));
        LOG_INFO("For field '{}' in db '{}' finded '{}' entity", field, meta.tableName, results.size());
        return results;
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
        QSqlQuery query(db.getThreadDatabase());
        query.prepare(QString("SELECT COUNT(1) FROM %1 WHERE id = ?")
                          .arg(QString::fromStdString(meta.tableName)));
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
        auto meta = Reflection<T>::meta();
        LOG_INFO("[repository] Truncate db '{}'", meta.tableName);
        QSqlQuery query(db.getThreadDatabase());
        QString sql = QString("DELETE FROM %1").arg(QString::fromStdString(meta.tableName)); // a очистити кеш?
        if (!query.exec())
            throw std::runtime_error(("Truncate failed: " + query.lastError().text()).toStdString());
        cache.clearPrefix(meta.tableName + ":");
    }

private:

    template<typename T>
    std::string makeKey(long long id) const {
        return std::string(Reflection<T>::meta().tableName) + ":" + std::to_string(id);
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
            if (std::string(f.name) == "id") continue;
            sets << QString("%1 = ?").arg(f.name);
            values << toVariant(f, entity);
        }
        return sets;
    }

    template<typename T>
    std::pair<QStringList, QStringList> buildInsertParts(const Meta& meta, const T& entity, QList<QVariant>& values) {
        QStringList cols, ph;
        for (const auto& f : meta.fields) {
            if (std::string(f.name) == "id") continue;
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
            return dt.isValid()
                       ? QVariant(dt.toSecsSinceEpoch())
                       : QVariant(QVariant::Int);
        }
        return {};
    }

    template<typename T>
    T buildEntity(QSqlQuery& query, const Meta& meta) const {
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
};

#endif // GENERICREPOSIROTY_H
