#include "GenericRepository.h"

#include "interfaces/ISqlExecutor.h"
#include "interfaces/IThreadPool.h"

GenericRepository::GenericRepository(ISqlExecutor *executor, ICacheService &cache, IThreadPool *pool,
                                     IOutboxWorker *outbox_worker)
    : executor_(executor), cache_(cache), pool_(pool), outbox_worker_(outbox_worker) {}

//GenericRepository::~GenericRepository() { outbox_worker_->stop(); }

void GenericRepository::clearCache() { cache_.clearCache(); }
