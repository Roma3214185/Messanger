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

class IDataBase {
public:
    virtual QSqlDatabase getThreadDatabase() = 0;
    virtual ~IDataBase() = default;
};

class SQLiteDatabase : public IDataBase {
public:
    explicit SQLiteDatabase(const QString& dbPath = "chat_db.sqlite")
        : dbPath(dbPath) {}

    QSqlDatabase getThreadDatabase() override {
        const QString connName = QString("connection_%1").arg((quintptr)QThread::currentThreadId());
        QSqlDatabase db;
        if (QSqlDatabase::contains(connName))
            db = QSqlDatabase::database(connName);
        else {
            db = QSqlDatabase::addDatabase("QSQLITE", connName);
            db.setDatabaseName(dbPath);
        }
        if (!db.isOpen() && !db.open())
            throw std::runtime_error("Cannot open database");

        return db;
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
        .get = [member](const void* obj) -> std::any{
            const T* element = static_cast<const T*>(obj);
            return element->*member;
        },
        .set = [member](void* obj, const std::any& val){
            T* element = static_cast<T*>(obj);
            if constexpr (std::is_same_v<M, std::string>) {
                if (val.type() == typeid(const char*))
                    (element->*member) = std::string(std::any_cast<const char*>(val));
                else
                    (element->*member) = std::any_cast<M>(val);
            } else {
                (element->*member) = std::any_cast<M>(val);
            }
        }
    };
}

template<typename T>
struct Reflection {
    static Meta meta();
};


template<typename T>
class GenericRepository {
    IDataBase& db;
    RedisCache& cache = RedisCache::instance();

public:
    explicit GenericRepository(IDataBase& db) : db(db) {}

    // ---------- CREATE / UPDATE ----------
    void save(T& entity) {
        QSqlDatabase conn = db.getThreadDatabase();
        QSqlQuery query(conn);
        auto meta = Reflection<T>::meta();

        const Field* idField = meta.find("id");
        if (!idField) throw std::runtime_error("Missing 'id' field in meta");

        long long id = std::any_cast<long long>(idField->get(&entity));
        bool isInsert = (id == 0);

        QString sql;
        QList<QVariant> values;

        if (isInsert) {
            auto [columns, placeholders] = buildInsertParts(meta, entity, values);
            sql = QString("INSERT INTO %1 (%2) VALUES (%3)")
                      .arg(QString::fromStdString(meta.tableName))
                      .arg(columns.join(", "))
                      .arg(placeholders.join(", "));
        } else {
            QStringList sets = buildUpdateParts(meta, entity, values);
            sql = QString("UPDATE %1 SET %2 WHERE id = ?")
                      .arg(QString::fromStdString(meta.tableName))
                      .arg(sets.join(", "));
            values << id;
        }

        query.prepare(sql);
        for (int i = 0; i < values.size(); ++i)
            query.bindValue(i, values[i]);

        if (!query.exec())
            throw std::runtime_error(("Save failed: " + query.lastError().text()).toStdString());

        if (isInsert) {
            QVariant newId = query.lastInsertId();
            if (newId.isValid()) idField->set(&entity, newId.toLongLong());
            cache.set(makeKey(getId(entity)), entity, std::chrono::hours(24));
        }
    }

    std::optional<T> findOne(long long id) {
        const std::string key = makeKey(id);
        if (auto cached = cache.get<T>(key)) {
            qDebug() << "[CACHE HIT] " << key;
            return cached;
        }

        QSqlQuery query(db.getThreadDatabase());
        auto meta = Reflection<T>::meta();
        query.prepare(QString("SELECT * FROM %1 WHERE id = ?")
                          .arg(QString::fromStdString(meta.tableName)));
        query.bindValue(0, id);

        if (!query.exec() || !query.next()) return std::nullopt;

        T entity = buildEntity(query, meta);
        cache.set(key, entity);
        return entity;
    }

    void deleteById(long long id) {
        auto meta = Reflection<T>::meta();
        QSqlQuery query(db.getThreadDatabase());
        query.prepare(QString("DELETE FROM %1 WHERE id = ?")
                          .arg(QString::fromStdString(meta.tableName)));
        query.bindValue(0, id);

        if (!query.exec())
            throw std::runtime_error(("Delete failed: " + query.lastError().text()).toStdString());

        cache.remove(makeKey(id));
    }


    std::vector<T> findBy(const std::string& field, const QVariant& value) {
        std::vector<T> results;
        auto meta = Reflection<T>::meta();

        if (!meta.find(field))
            throw std::invalid_argument("Invalid field: " + field);

        QSqlQuery query(db.getThreadDatabase());
        query.prepare(QString("SELECT * FROM %1 WHERE %2 = ?")
                          .arg(QString::fromStdString(meta.tableName))
                          .arg(QString::fromStdString(field)));
        query.bindValue(0, value);

        if (!query.exec()) return results;
        while (query.next()) results.push_back(buildEntity(query, meta));
        return results;
    }

    bool exists(long long id) {
        const std::string key = makeKey(id);
        if (cache.exists(key)) {
            qDebug() << "[CACHE HIT] true" << key;
            return true;
        }

        auto meta = Reflection<T>::meta();
        QSqlQuery query(db.getThreadDatabase());
        query.prepare(QString("SELECT COUNT(1) FROM %1 WHERE id = ?")
                          .arg(QString::fromStdString(meta.tableName)));
        query.bindValue(0, id);

        if (!query.exec() || !query.next()) return false;
        bool found = query.value(0).toLongLong() > 0;
        if (found) cache.set(key, true, std::chrono::minutes(5));
        return found;
    }

    void truncate() {
        auto meta = Reflection<T>::meta();
        QSqlQuery query(db.getThreadDatabase());
        QString sql = QString("DELETE FROM %1").arg(QString::fromStdString(meta.tableName)); // a очистити кеш?
        if (!query.exec())
            throw std::runtime_error(("Truncate failed: " + query.lastError().text()).toStdString());
        cache.clearPrefix(meta.tableName + ":");
    }

private:

    std::string makeKey(long long id) const {
        return std::string(Reflection<T>::meta().tableName) + ":" + std::to_string(id);
    }

    long long getId(const T& obj) const {
        auto meta = Reflection<T>::meta();
        if (auto f = meta.find("id"))
            return std::any_cast<long long>(f->get(&obj));
        return 0;
    }

    QStringList buildUpdateParts(const Meta& meta, const T& entity, QList<QVariant>& values) {
        QStringList sets;
        for (const auto& f : meta.fields) {
            if (std::string(f.name) == "id") continue;
            sets << QString("%1 = ?").arg(f.name);
            values << toVariant(f, entity);
        }
        return sets;
    }

    std::pair<QStringList, QStringList> buildInsertParts(const Meta& meta, const T& entity, QList<QVariant>& values) {
        QStringList cols, ph;
        for (const auto& f : meta.fields) {
            if (std::string(f.name) == "id") continue;
            cols << f.name;
            ph << "?";
            values << toVariant(f, entity);
        }
        return {cols, ph};
    }

    QVariant toVariant(const Field& f, const T& entity) const {
        std::any val = f.get(&entity);
        if (f.type == typeid(long long)) return QVariant::fromValue(std::any_cast<long long>(val));
        if (f.type == typeid(std::string)) return QString::fromStdString(std::any_cast<std::string>(val));
        if (f.type == typeid(QDateTime)) return std::any_cast<QDateTime>(val);
        return {};
    }

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
