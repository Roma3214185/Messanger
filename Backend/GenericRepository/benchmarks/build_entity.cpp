#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include "benchmark/benchmark.h"
#include "GenericReposiroty.h"
#include "Meta.h"
#include "MessageService/Headers/Message.h"
#include "IEntityBuilder.h"

QSqlDatabase getBenchmarkDb() {
    static bool initialized = false;
    static QSqlDatabase db;

    if (!initialized) {
        db = QSqlDatabase::addDatabase("QSQLITE", "benchmark_conn");
        db.setDatabaseName(":memory:");
        db.open();

        QSqlQuery q(db);
        q.exec(R"(CREATE TABLE messages (
            id INTEGER,
            chat_id INTEGER,
            sender_id INTEGER,
            text TEXT,
            timestamp INTEGER
        );)");

        db.transaction();
        for (int i = 0; i < 1000; ++i) {
            q.prepare("INSERT INTO messages (id, chat_id, sender_id, text, timestamp) VALUES (?, ?, ?, ?, ?)");
            q.addBindValue(i);
            q.addBindValue(i % 10);
            q.addBindValue(i % 50);
            q.addBindValue(QString("Message %1").arg(i));
            q.addBindValue(1000000 + i);
            q.exec();
        }
        db.commit();

        initialized = true;
    }

    return db;
}


static void BM_BuildEntity_Dynamic(benchmark::State& state) {
    QSqlDatabase db = getBenchmarkDb();
    QSqlQuery query(db);
    query.exec("SELECT id, chat_id, sender_id, text, timestamp FROM messages;");
    auto builder = makeBuilder<Message>(BuilderType::Meta);

    for (auto _ : state) {
        query.first();
        std::vector<Message> results;
        while (query.next()) {
            results.push_back(builder->build(query));
        }
        benchmark::DoNotOptimize(results);
    }
}

static void BM_BuildEntity_Static(benchmark::State& state) {
    QSqlDatabase db = getBenchmarkDb();
    QSqlQuery query(db);
    query.exec("SELECT id, chat_id, sender_id, text, timestamp FROM messages;");

    for (auto _ : state) {
        query.first();
        std::vector<Message> results;
        while (query.next()) {
            Message entity;
            entity.id = query.value(0).toInt();
            entity.chat_id = query.value(1).toInt();
            entity.sender_id = query.value(2).toInt();
            entity.text = query.value(3).toString().toStdString();
            entity.timestamp = query.value(4).toInt();
            results.push_back(entity);
        }
        benchmark::DoNotOptimize(results);
    }
}

static void BM_BuildEntity_Fast(benchmark::State& state) {
    QSqlDatabase db = getBenchmarkDb();
    QSqlQuery query(db);
    query.exec("SELECT id, chat_id, sender_id, text, timestamp FROM messages;");
    auto builder = makeBuilder<Message>(BuilderType::Fast);

    for (auto _ : state) {
        query.first();
        std::vector<Message> results;
        while (query.next()) {
            results.push_back(builder->build(query));
        }
        benchmark::DoNotOptimize(results);
    }
}

static void BM_GenericBuildEntity_Fast(benchmark::State& state) {
    QSqlDatabase db = getBenchmarkDb();
    QSqlQuery query(db);
    query.exec("SELECT id, chat_id, sender_id, text, timestamp FROM messages;");
    auto builder = makeBuilder<Message>(BuilderType::Generic);

    for (auto _ : state) {
        query.first();
        std::vector<Message> results;
        while (query.next()) {
            results.push_back(builder->build(query));
        }
        benchmark::DoNotOptimize(results);
    }
}

BENCHMARK(BM_BuildEntity_Dynamic);
BENCHMARK(BM_BuildEntity_Static);
BENCHMARK(BM_BuildEntity_Fast);
BENCHMARK(BM_GenericBuildEntity_Fast);
