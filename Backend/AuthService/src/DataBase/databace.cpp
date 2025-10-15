#include "database.h"
#include <string>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QThread>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDatabase>
#include <QSqlRecord>

template<typename... Args>
bool DataBase::executeQuery(QSqlQuery& query, Args&&... args) {
    auto toVariant = [](auto&& value) -> QVariant {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return QVariant(QString::fromStdString(value));
        } else {
            return QVariant(std::forward<decltype(value)>(value));
        }
    };

    (query.addBindValue(toVariant(std::forward<Args>(args))), ...);
    bool success = query.exec();
    if (!success) {
        qDebug() << "[ERROR] Query failed:" << query.lastQuery();
        qDebug() << "[ERROR] Error:" << query.lastError().text();
    }else{
        qDebug() << "[INFO] Query succeeded:" << query.lastQuery();
    }
    return success;
}

DataBase::DataBase(){
    createUserDataBase();
}

QSqlDatabase DataBase::getThreadDatabase() {
    const QString connName = QString("connection_%1").arg((quintptr)QThread::currentThreadId());

    QSqlDatabase db;
    if (QSqlDatabase::contains(connName)) {
        db = QSqlDatabase::database(connName);
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", connName);
        db.setDatabaseName("chat_db");
    }

    if (!db.isOpen()) {
        if (!db.open()) {
            qCritical() << "[ERROR] Cannot open DB in thread:"
                        << QThread::currentThread()
                        << db.lastError().text();

            throw std::runtime_error("Cannot open database");
        }
    }

    return db;
}

User DataBase::getUserFromQuery(const QSqlQuery& query) {
    User user;
    user.id = query.value("id").toInt();
    user.username = query.value("username").toString().toStdString();
    user.email = query.value("email").toString().toStdString();
    user.tag = query.value("tag").toString().toStdString();
    return user;
}

void DataBase::createUserDataBase(){
    auto db = getThreadDatabase();
    QSqlQuery query(db);
    query.prepare(R"(
        CREATE TABLE IF NOT EXISTS users(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            email TEXT NOT NULL,
            tag TEXT NOT NULL,
        );
    )");

    executeQuery(query);
}

OptionalUser DataBase::createUser(RegisterRequest req){
    auto db = getThreadDatabase();
    QSqlQuery query(db);
    query.prepare("INSERT INTO users (name, email, password, tag) VALUES (?, ?, ?, ?);");

    if(!executeQuery(query, req.name, req.email, req.password, req.tag)){
        return std::nullopt;
    }

    int newId = query.lastInsertId().toInt();
    return User{
        .username = req.name,
        .email = req.email,
        .id = newId
    };
}

OptionalUser DataBase::findByEmail(std::string email){
    auto db = getThreadDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT id, name, email, tag FROM users WHERE email = ?");

    if(!executeQuery(query, email) || !query.next()){
        return std::nullopt;
    }

    return getUserFromQuery(query);
}

OptionalUser DataBase::findById(int id){
    auto db = getThreadDatabase();
    QSqlQuery query(db);
    query.prepare("SELECT id, name, email, tag FROM users WHERE id = ?");

    if(!executeQuery(query, id) || !query.next()) {
        return std::nullopt;
    }

    return getUserFromQuery(query);
}

QList<User> DataBase::findByTag(std::string tag){
    auto db = getThreadDatabase();
    QSqlQuery query(db);

    query.prepare("SELECT id, name, email, tag, password FROM users WHERE tag=?");

    if(!executeQuery(query, tag)) {
        return {};
    }

    QList<User> res;
    while(query.next()){
        res.push_back(getUserFromQuery(query));
    }

    return res;
}

void DataBase::clear() {
    auto db = getThreadDatabase();
    QSqlQuery query(db);
    query.prepare("DELETE FROM users;");

    executeQuery(query);
}

