#include <iostream>
#include <syncstream>
#include <vector>
#include <map>
#include <thread>
#include <barrier>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

const int nt = 6;

const vector<pair<char, int>> totals = {
    {'a', 6}, {'b', 7}, {'c', 9}, {'d', 8}, {'e', 5}, {'f', 9}, {'g', 7}, {'h', 4}, {'i', 9}, {'j', 6}
};

const vector<string> jobs = {
    "abcdef", // 1a 1b 1c 1d 1e 1f
    "abcdef", // 1a 1b 1c 1d 1e 1f
    "abcdef", // 1a 1b 1c 1d 1e 1f
    "abcdef", // 1a 1b 1c 1d 1e 1f
    "abcdef", // 1a 1b 1c 1d 1e 1f
    "abcdff", // 1a 1b 1c 1d 2f
    "bccdff", // 1b 2c 1d 2f
    "cdgghi", // 1c 1d 2g 1h 1i
    "gghiii", // 2g 1h 3i
    "ghiiii", // 1g 1h 4i
    "ghijjj", // 1g 1h 1i 3j                 
    "jjj"     // 3j
};

void f(char x, int i) {
    osyncstream(cout) << "З набору " << x << " виконано дію " << i << "." << endl;
}

map<char, atomic<int>> init_counters() {
    map<char, atomic<int>> counters;

    for (const auto& p : totals) {
        counters[p.first] = 0;
    }

    return counters;
}

void run_simulation() {
    auto next_idx = init_counters();

    barrier sync(nt);

    vector<jthread> workers;
    workers.reserve(nt);

    for (int tid = 0; tid < nt; ++tid) {
        workers.emplace_back([&, tid] {
            for (const auto& job : jobs) {
                if (tid < job.size()) {
                    int i = next_idx[job[tid]].fetch_add(1) + 1;
                    f(job[tid], i);
                }
                sync.arrive_and_wait();
            }
            });
    }
}

int main() {

#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif

    osyncstream(cout) << "Обчислення розпочато." << endl;

    run_simulation();

    osyncstream(cout) << "Обчислення завершено." << endl;
}
