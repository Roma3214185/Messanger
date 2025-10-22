# GenericRepository Benchmarks

This project implements a **GenericRepository** for working with **SQLiteDatabase** and demonstrates the use of **Entity Cache** to speed up data access.

---

## Overview

The goal of this project is to measure the performance benefits of entity caching for different types of data access:

1. **Query** — queries with field filters.
2. **Entity** — direct lookup by ID.
3. **Entity Cache** — caching entities to speed up repeated access.

Benchmarks are implemented using [Google Benchmark](https://github.com/google/benchmark) and [spdlog](https://github.com/gabime/spdlog) for logging.

---

## Project Structure

```
GenericRepository/
├─ benchmarks/
│ ├─ main.cpp # Single main() for all benchmarks
│ ├─ benchmarks_query.cpp # Benchmarks for query operations
│ ├─ benchmark_entity.cpp # Benchmarks for entity operations
│ ├─ README.md
│
├─ GenericRepository.h/cpp
├─ Query.h/cpp
├─ SQLiteDatabase.h/cpp
```

- `main.cpp` initializes **Qt**, the global database, and runs all benchmarks.  
- All `spdlog` output is either disabled.

---

## Benchmark Results

| Benchmark                   | Time (ns) | CPU (ns) | Iterations |
|------------------------------|-----------|----------|------------|
| QueryWithoutCache            | 162,492   | 67,970   | 100        |
| QueryWithCache               | 17,172    | 16,910   | 100        |
| EntityWithoutCache           | 67,461    | 29,160   | 100        |
| EntityWithCache              | 22,496    | 21,940   | 100        |

---

### Analysis

1. **Cache Effectiveness**  
   - Query: ~9.5× faster  
   - Entity: ~3× faster  
   - Entity cache significantly reduces database access time.

2. **Query vs Entity**  
   - Direct lookup by ID (`Entity`) is faster than query without cache (~3×), because queries involve extra filtering and parsing.  
   - With cache, both queries and direct lookups become very fast and close in performance.

3. **CPU vs Time**  
   - For `QueryWithoutCache`, Time is much higher than CPU → shows I/O and parsing overhead.  
   - For cached operations, Time ≈ CPU → cache removes most I/O overhead.

---

## Conclusions

- Entity caching is **crucial for speeding up data access**.  
- The biggest benefits are seen on **repeated accesses to the same data**.  
- For complex queries, enabling a **Query Cache** can further increase performance.

---

## Visualization

A simple textual visualization of speed improvements:
```
QueryWithoutCache: ████████████████████ 162,492 ns
QueryWithCache: ███ 17,172 ns
EntityWithoutCache: ███████ 67,461 ns
EntityWithCache: ██ 22,496 ns
```


Shorter bars → faster execution. Clearly shows the significant performance gain from caching, especially for queries.

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
