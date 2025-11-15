#include "GenericRepository.h"

GenericRepository::GenericRepository(IDataBase&     database,
                                     ISqlExecutor&  executor,
                                     ICacheService& cache,
                                     ThreadPool*    pool)
    : database_(database), executor_(executor), cache_(cache), pool_(pool) {}

void GenericRepository::clearCache() { cache_.clearCache(); }
