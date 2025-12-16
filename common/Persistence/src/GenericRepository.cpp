#include "GenericRepository.h"
#include "interfaces/IThreadPool.h"
#include "interfaces/IIdGenerator.h"
#include "interfaces/ISqlExecutor.h"

GenericRepository::GenericRepository(IDataBase&     database,
                                     ISqlExecutor*  executor,
                                     ICacheService& cache,
                                     IIdGenerator* generator,
                                     IThreadPool*    pool)
    : database_(database)
    , executor_(executor)
    , cache_(cache)
    , generator_(generator)
    , pool_(pool)
    , outbox_worker_(std::make_unique<OutboxWorker>(database))  {
  outbox_worker_->start();
}

GenericRepository::~GenericRepository() {
  outbox_worker_->stop();
}

void GenericRepository::clearCache() { cache_.clearCache(); }
