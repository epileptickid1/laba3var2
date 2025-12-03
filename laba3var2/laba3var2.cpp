#include <iostream>
#include <thread>
#include <latch>
#include <syncstream>
#include <chrono>

using namespace std::chrono_literals;

void f(const std::string& x, int i) {
    std::osyncstream out(std::cout);
    out << "З набору " << x << " виконано дію " << i << "." << std::endl;
    std::this_thread::sleep_for(80ms);
}

struct TaskSet {
    std::string name;
    int count;
    int id;

    int main() {

        const int nt = 4;


        std::vector<TaskSet> sets = {
            {"a", 4,  0},
            {"b", 8,  1},
            {"c", 6,  2},
            {"d", 6,  3},
            {"e", 7,  4},
            {"f", 7,  5},
            {"g", 6,  6},
            {"h", 7,  7},
            {"i", 5,  8},
            {"j", 9,  9},
            {"k", 4, 10}
        };
        const int N = (int)sets.size();


        std::vector<std::vector<int>> dependents(N);
        std::vector<int> dep_count(N, 0);

        auto add_edge = [&](int from, int to) {
            dependents[from].push_back(to);
            dep_count[to] += 1;
            };


        add_edge(0, 3);
        add_edge(0, 4);
        add_edge(1, 5);
        add_edge(2, 5);
        add_edge(5, 8);
        add_edge(4, 8);
        add_edge(3, 6);
        add_edge(3, 7);
        add_edge(7, 10);
        add_edge(8, 10);


        std::vector<std::unique_ptr<std::latch>> latches;
        latches.reserve(N);
        for (int i = 0; i < N; ++i) {
            latches.emplace_back(std::make_unique<std::latch>(dep_count[i]));
        }


        std::vector<std::vector<int>> assignments(nt);
        assignments[0] = { 9, 0, 8 };
        assignments[1] = { 1, 6,10 };
        assignments[2] = { 4, 3, 2 };
        assignments[3] = { 5, 7 };

        {
            std::vector<bool> used(N, false);
            for (int t = 0; t < nt; ++t) {
                for (int id : assignments[t]) {
                    if (id >= 0 && id < N) used[id] = true;
                }
            }

            for (int i = 0; i < N; ++i) {
                if (!used[i]) {
                    std::osyncstream out(std::cout);
                    out << "Warning: set " << sets[i].name << " (id=" << i << ") not assigned to any worker!" << std::endl;
                }
            }
        }


        {
            std::osyncstream out(std::cout);
            out << "Обчислення розпочато." << std::endl;
        }

        auto t_start = std::chrono::steady_clock::now();


        std::vector<std::thread> workers;
        workers.reserve(nt);

        for (int t = 0; t < nt; ++t) {
            workers.emplace_back([t, &assignments, &sets, &latches, &dependents]() {

                for (int set_id : assignments[t]) {

                    latches[set_id]->wait();

                    for (int i = 1; i <= sets[set_id].count; ++i) {
                        f(sets[set_id].name, i);
                    }

                    for (int dep : dependents[set_id]) {
                        latches[dep]->count_down();
                    }
                }
                });
        }

        for (auto& th : workers) if (th.joinable()) th.join();

        auto t_end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = t_end - t_start;


        {
            std::osyncstream out(std::cout);
            out << "Обчислення завершено." << std::endl;
            out << "Загальний час (сек): " << elapsed.count() << std::endl;
        }
    }
};