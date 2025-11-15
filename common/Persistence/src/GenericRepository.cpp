#include "GenericRepository.h"
#include "interfaces/IThreadPool.h"

GenericRepository::GenericRepository(IDataBase&     database,
                                     ISqlExecutor*  executor,
                                     ICacheService& cache,
                                     IThreadPool*    pool)
    : database_(database), executor_(executor), cache_(cache), pool_(pool) {}

void GenericRepository::clearCache() { cache_.clearCache(); }
