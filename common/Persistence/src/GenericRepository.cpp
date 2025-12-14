#include "GenericRepository.h"
#include "interfaces/IThreadPool.h"
#include "interfaces/IIdGenerator.h"

GenericRepository::GenericRepository(IDataBase&     database,
                                     ISqlExecutor*  executor,
                                     ICacheService& cache,
                                     IIdGenerator* generator,
                                     IThreadPool*    pool)
    : database_(database), executor_(executor), cache_(cache), generator_(generator), pool_(pool) {}

void GenericRepository::clearCache() { cache_.clearCache(); }
