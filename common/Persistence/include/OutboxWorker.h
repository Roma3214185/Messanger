#ifndef OUTBOXWORKER_H
#define OUTBOXWORKER_H

#include <QDebug>
#include <QMutex>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QThread>
#include <QWaitCondition>
#include <nlohmann/json.hpp>

#include "Debug_profiling.h"
#include "entities/User.h"
#include "interfaces/IDataBase.h"

class IOutboxWorker {
 public:
  virtual ~IOutboxWorker() = default;
};

class OutboxWorker : public IOutboxWorker, QThread {
  // Q_OBJECT
 public:
  explicit OutboxWorker(IDataBase &db);

  ~OutboxWorker() override;

  OutboxWorker(const OutboxWorker &) = delete;
  OutboxWorker(OutboxWorker &&) = delete;
  OutboxWorker &operator=(const OutboxWorker &) = delete;
  OutboxWorker &operator=(OutboxWorker &&) = delete;

  void startWorker();

  void stop();

 protected:
  void run() override;

 private:
  IDataBase &db_;
  volatile bool stop_{false};
  QMutex mutex_;
  QWaitCondition wait_condition_;

  void processBatch(IDataBase &db);
};

#endif  // OUTBOXWORKER_H
