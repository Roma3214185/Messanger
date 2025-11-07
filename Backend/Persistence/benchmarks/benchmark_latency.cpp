#include <QCoreApplication>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>

// #include "Persistence/GenericRepository.h"

// class MessagePool {
//  public:
//   Message* acquire() {
//     std::lock_guard<std::mutex> lock(mutex_);
//     if (!pool_.empty()) {
//       Message* msg = pool_.front();
//       pool_.pop();
//       return msg;
//     }
//     return new Message();
//   }

//   void release(Message* msg) {
//     // msg->reset();  // reset fields (id=0, text.clear(), etc.)
//     std::lock_guard<std::mutex> lock(mutex_);
//     pool_.push(msg);
//   }

//   ~MessagePool() {
//     while (!pool_.empty()) {
//       delete pool_.front();
//       pool_.pop();
//     }
//   }

//  private:
//   std::queue<Message*> pool_;
//   std::mutex mutex_;
// };

// template <typename Func>
// void measureLatency(Func sendFunc, int iterations = 1000) {
//   std::vector<double> latencies_ms;
//   latencies_ms.reserve(iterations);

//   for (int i = 0; i < iterations; ++i) {
//     auto start = std::chrono::high_resolution_clock::now();
//     sendFunc();  // your message sending function
//     auto end = std::chrono::high_resolution_clock::now();

//     double ms = std::chrono::duration<double, std::milli>(end - start).count();
//     latencies_ms.push_back(ms);
//   }

//   std::sort(latencies_ms.begin(), latencies_ms.end());

//   auto p95 = latencies_ms[size_t(iterations * 0.95)];
//   auto p99 = latencies_ms[size_t(iterations * 0.99)];
//   auto avg = std::accumulate(latencies_ms.begin(), latencies_ms.end(), 0.0) /
//              iterations;

//   std::cout << "Average: " << avg << " ms\n";
//   std::cout << "P95: " << p95 << " ms\n";
//   std::cout << "P99: " << p99 << " ms\n";
// }

int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);

//   SQLiteDatabase db;
//   GenericRepository repo(db);
//   MessagePool pool;
//   ThreadPool tp(4);

//   int iterations = 10000;

//   measureLatency(
//       [&]() {
//         Message msg;
//         msg.text = "Hello world";

//         auto fut = tp.enqueue([&msg, &repo, &pool]() {
//           repo.save(msg);
//           // pool.release(msg);
//         });

//         fut.get();
//       },
//       iterations);

//   return 0;
}

// // cmake .. -DCMAKE_BUILD_TYPE=Release
// // cmake --build . --target latencies
