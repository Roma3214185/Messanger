# GenericRepository Benchmarks

This project implements a `GenericRepository` for working with `SQLiteDatabase` and demonstrates the use of **Entity Cache**, **Query Cache**, and **Redis pipeline** to speed up data access.  
Additionally, it benchmarks **different entity-building strategies** from `QSqlQuery` rows.

## Overview

The goal of this project is to measure performance benefits of different data access strategies:

- **Query** — fetching data with field filters.
- **Entity** — direct lookup by ID.
- **Entity Cache** — caching entities to speed up repeated access.
- **Async / Thread Pool** — running database queries asynchronously.
- **Redis pipeline** — bulk saving entities to Redis efficiently.
- **Entity Build** — constructing entities from database rows.
- **Prepared Query Cache** — reusing `QSqlQuery` objects to avoid repeated preparation.

Benchmarks are implemented using **Google Benchmark** and **spdlog** for logging.

## Project Structure

```
GenericRepository/
├─ benchmarks/
│ ├─ main.cpp # Single main() for all benchmarks
│ ├─ benchmarks_query.cpp # Benchmarks for query operations
│ ├─ benchmark_entity.cpp # Benchmarks for entity operations
│ ├─ README.md
│
├─ GenericRepository.h
├─ Query.h/cpp
├─ SQLiteDatabase.h
├─ ThreadPool.h
├─ Meta.h
├─ IEntityBuilder.h
```


`main.cpp` initializes Qt, the global database, and runs all benchmarks.  
All `spdlog` output during benchmarks is either disabled or redirected to a file.

## Benchmark Results

### Query Benchmarks

| Benchmark | Time (ns) | CPU (ns) | Iterations |
|-----------|-----------|-----------|------------|
| QueryWithoutCache | 194,274 | 82,760 | 100 |
| QueryWithCache    | 18,200  | 17,960 | 100 |
| QueryWithCacheAsync | 183,825 | 7,240 | 100 |
| QueryWithoutCacheAsync | 55,505 | 6,500 | 100 |

### Entity Benchmarks

| Benchmark | Time (ns) | CPU (ns) | Iterations |
|-----------|-----------|-----------|------------|
| EntityWithoutCache | 146,840 | 74,700 | 100 |
| EntityWithCache    | 52,972  | 41,140 | 100 |
| EntityWithCacheAsync | 179,022 | 7,620 | 100 |

### Redis Save Benchmarks

| Benchmark | 10 entities | 100 entities | 1000 entities |
|-----------|------------|-------------|---------------|
| Save Individually | 418,593 ns | 4,204,811 ns | 42,109,232 ns |
| Save with Pipeline | 274,070 ns | 2,236,466 ns | 21,502,009 ns |

### Entity Build Benchmarks

| Benchmark | Time (ns) | CPU (ns) | Iterations |
|-----------|-----------|-----------|------------|
| BM_BuildEntity_Dynamic | 707,195 | 692,101 | 974 |
| BM_BuildEntity_Static  | 239,487 | 235,410 | 2,835 |
| BM_BuildEntity_Fast    | 15,642  | 15,267  | 48,653 |
| BM_GenericBuildEntity_Fast | 159,064 | 158,904 | 4,540 |

### Query Preparation Benchmarks

| Benchmark                  | Time (ns) | Iterations |
|----------------------------|-----------|------------|
| PrepareQuery               | 4,532–5,076 | 160,170    |
| PrepareQueryWithCache      | 345–378    | 2,030,575  |

## Analysis

### Cache Effectiveness

- **Query:** ~6× faster with cache.  
- **Entity:** ~4× faster with cache.  
- Entity caching significantly reduces database access time, especially on repeated accesses.

### Thread Pool / Async Effects

- Async/thread pool can **reduce blocking on slow queries**.  
- For small, cached queries, async **adds overhead**, so wall time increases compared to synchronous cache access.  
- Combining **cache + thread pool** shows intermediate performance: caching reduces I/O, but thread scheduling overhead still adds wall time.

### Entity Build Analysis

- **Dynamic builder (`Meta + std::any`)** is slowest (~707 ns), due to runtime type erasure and indirect field assignments.  
- **Static template builder** improves performance (~239 ns), because field types are known at compile time.  
- **Hand-written fast builder** (`assign(e.id); ...`) is extremely fast (~15 ns) — essentially zero-overhead, inlined assignments.  
- **Generic tuple-based `FastBuilder`** is faster than dynamic (~191 ns) but slightly slower than hand-written fast builder because of `std::apply` + lambda overhead.  
- **Takeaway:** Hand-written or fully inlined lambda assignment per entity gives maximum performance. Tuple-based generic builders are still significantly faster than dynamic but slightly slower than hand-coded.

### Redis Pipeline

- **Pipeline** reduces time by ~30–50% for bulk inserts.  
- Individual `SET` commands incur a round-trip for each entity → slower.  
- For small numbers of entities (≤10), overhead of pipeline ≈ minimal benefit.  
- For large batches (≥100), pipeline is strongly recommended.

### CPU vs Time

- For uncached queries, `Time >> CPU` → indicates I/O and parsing overhead.  
- For cached operations, `Time ≈ CPU` → cache removes most I/O overhead.  
- Redis pipeline reduces network I/O, so `CPU < Time` is improved.

### Query Preparation
- `PrepareQuery`: every iteration constructs a new `QSqlQuery` and calls `prepare()`.  
- `PrepareQueryWithCache`: caches prepared statements per-thread. Once prepared, the query is reused, resulting in **~12× faster execution**.  


## Relative Speedup (vs No Cache Sync)

| Operation                       | Speedup × |
|---------------------------------|------------|
| QueryWithoutCache                | 1×         |
| QueryWithCache                   | 6×         |
| QueryWithoutCacheAsync           | 2.6×       |
| QueryWithCacheAsync              | 0.84×      |
| EntityWithoutCache               | 1.01×      |
| EntityWithCache                  | 4×         |
| EntityWithoutCacheAsync          | 1.53×      |
| EntityWithCacheAsync             | 0.93×      |

**Interpretation:**  
- **Cache** provides the largest speedup (up to 6× for queries).  
- **Async/thread pool** alone helps moderately for uncached operations (2–1.5×).  
- **Cache + Async** is slightly slower than synchronous cache due to thread scheduling overhead.

---

## Visualization

A simple textual visualization of speed improvements:

**Query (Sync vs Async)**
```
QueryWithoutCache: ████████████████████ 185,963 ns
QueryWithCache: ███ 30,897 ns
QueryWithCacheAsync: █████ 221,593 ns
QueryWithoutCacheAsync: ████ 71,614 ns
```
**Entity (Sync vs Async)**
```
EntityWithoutCache: █████████████████ 183,172 ns
EntityWithCache: ████ 45,252 ns
EntityWithCacheAsync: █████ 200,302 ns
EntityWithoutCacheAsync: ████ 120,624 ns
```
**Redis Pipeline (Bulk Save)**
```
Save Individually (1000): ██████████████████████████████ 42,109,232 ns
Save Pipeline (1000): ██████████████████ 21,502,009 ns
```
**Entity Build**
```
BM_BuildEntity_Dynamic: █████████████████████████████████████████ 707,195 ns
BM_BuildEntity_Static: ██████████████ 239,487 ns
BM_BuildEntity_Fast: █ 15,642 ns
BM_GenericBuildEntity_Fast: ██████ 191,064 ns
```
**Query Preparation**
```
PrepareQuery:           █████████████████████████████████████████████████████████  4,532 ns  
PrepareQueryWithCache:  ███  345 ns
```


**Legend / Insights:**  
- **Shorter bars → faster execution**.  
- **Cache vs No Cache:** clearly shows major speedup (~4–6×).  
- **Async / Thread Pool:** introduces wall time overhead for fast cached operations.  
- **Cache + Async:** still slower than synchronous cache, but can help for heavy queries in parallel.  
- **Redis Pipeline:** reduces bulk insert time ~30–50%, more effective for ≥100 entities.  
- **Cache + Async / Pipeline:** best performance for high-throughput or parallel workloads.
- **Dynamic build** is the slowest.  
- **Static template build** is ~3× faster.  
- **Hand-written fast build** is ~45× faster than dynamic.  
- **Generic FastBuilder** is ~3–4× faster than dynamic but slower than hand-written fast.  
- **Takeaway:** Fast inlined entity builders drastically reduce construction time.
- **PrepareQuery:** Every call creates a new QSqlQuery and calls .prepare() again.
- **PrepareQueryWithCache:** Reuses an existing prepared query stored in a map, avoiding re-preparation.

## Usage

Clone the repository and build benchmarks:

```bash
git clone <repo_url>
cd GenericRepository
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target benchmarks
./benchmarks
```

## Logging
spdlog output during benchmarks is disabled or redirected to a file (bench_log.txt) to keep the console and tables clean.
For debugging, logging can be enabled via spdlog::set_level(spdlog::level::debug) outside of benchmark loops


## Conclusions
- **Entity caching** is the most effective way to speed up repeated access (~4–6×).  
- **Thread pool / Async** is useful only for expensive DB queries; adds overhead for fast cached operations.  
- **Redis Pipeline** is highly beneficial for bulk inserts (~30–50% faster than individual SETs).  
- **Best performance**: combine **cache + pipeline** for high-throughput inserts; use **cache without async** for frequent small reads.
- **Entity Build**: hand-written / inlined builders are ~50× faster than dynamic, tuple-based generic builders are ~3–4× faster than dynamic.
- **PrepareQueryWithCache**: Query preparation became ~12–15× faster.
