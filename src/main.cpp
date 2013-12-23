#include "btree.hpp"
#include <chrono>
#include <iostream>
#include <functional>
#include <set>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::cout;
using std::endl;

long timeit(std::function<void(void)> f) {
  high_resolution_clock c;
  high_resolution_clock::time_point start = c.now();
  f();
  high_resolution_clock::time_point end = c.now();
  auto t = duration_cast<milliseconds>(end - start).count();
  return t;
}

void insertion_btree_benchmark() {
  btree<5, long long int> b;
  const long long int n = 4000000;
  const long long int p = 8000009;
  for (long long int i = 0; i < n; ++i) {
    long long int x = i * i % p;
    b.insert(x);
  }
}

void insertion_set_benchmark() {
  std::set<long long int> s;
  const long long int n = 4000000;
  const long long int p = 8000009;
  for (long long int i = 0; i < n; ++i) {
    long long int x = i * i % p;
    s.insert(x);
  }
}

int main() {
  long t = timeit(insertion_btree_benchmark);
  cout << "Insertion into B-tree took " << t << " milliseconds." << endl;
  t = timeit(insertion_set_benchmark);
  cout << "Insertion into std::set took " << t << " milliseconds." << endl;
}
