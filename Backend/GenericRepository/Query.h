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
    Query(QSqlDatabase db, const std::string& table)
        : db(std::move(db)), tableName(table) {}

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
        std::vector<T> results;
        auto meta = Reflection<T>::meta();

        QString sql = QString("SELECT * FROM %1").arg(QString::fromStdString(tableName));
        if (!filters.empty())
            sql += " WHERE " + filters.join(" AND ");
        if (!order.isEmpty())
            sql += " " + order;
        if (!limitClause.isEmpty())
            sql += " " + limitClause;

        LOG_INFO("SQL to execute sql '{}'", sql.toStdString());
        LOG_INFO("Values to add '{}'", values.size());
        QSqlQuery query(db);
        query.prepare(sql);

        for (int i = 0; i < values.size(); ++i){
            query.bindValue(i, values[i]);
            //LOG_DEBUG("Value s '{}'", values[i]);
        }

        if (!query.exec()) {
            LOG_ERROR("Query error: {}", query.lastError().text().toStdString());
            return results;
        }

        while (query.next())
            results.push_back(buildEntity(query, meta));
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

private:
    QSqlDatabase db;
    QStringList filters;
    QVector<QVariant> values;
    QString order;
    QString limitClause;
    std::string tableName;
};

#endif // QUERY_H
