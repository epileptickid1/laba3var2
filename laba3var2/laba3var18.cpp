#include <iostream>
#include <thread>
#include <latch>
#include <syncstream>
#include <vector>
#include <functional>
#include <windows.h>




const int A_COUNT = 8;
const int B_COUNT = 5;
const int C_COUNT = 5;
const int D_COUNT = 5;
const int E_COUNT = 5;
const int F_COUNT = 7;
const int G_COUNT = 4;
const int H_COUNT = 4;
const int I_COUNT = 5;
const int J_COUNT = 9;

const int NT = 5;


void f(char task_name, int action_num, std::osyncstream& out) {
    out << "from set " << task_name
        << " did action " << action_num << ".\n";
    out.emit();
}

void run_task(char name, int count, std::osyncstream& out) {
    for (int i = 1; i <= count; ++i)
        f(name, i, out);
}

int main() {
    
    SetConsoleOutputCP(CP_UTF8);
    std::osyncstream out(std::cout);
    out << "calc start.\n";
    out.emit();

    
    std::latch latch_a(1);
    std::latch latch_cb(2);
    std::latch latch_d(1);
    std::latch latch_final(6);

    auto t1 = [&](std::stop_token) {
        run_task('a', A_COUNT, out);
        latch_a.count_down();

        //latch_a.wait();
        run_task('e', E_COUNT, out);
        latch_final.count_down();
        };

   
    auto t2 = [&](std:: stop_token) {
        run_task('b', B_COUNT, out);
        latch_cb.count_down();

        latch_cb.wait();
        run_task('g', G_COUNT, out);
        latch_final.count_down();
        };

   
    auto t3 = [&](std:: stop_token) {
        run_task('c', C_COUNT, out);
        latch_cb.count_down();

        latch_cb.wait();
        run_task('h', H_COUNT, out);
        latch_final.count_down();
        };

 
    auto t4 = [&](std::stop_token) {
        run_task('d', D_COUNT, out);
        latch_d.count_down();

        //latch_d.wait();
        run_task('i', I_COUNT, out);
        latch_final.count_down();
        };

   
    auto t5 = [&](std::stop_token) {
        latch_a.wait();
        run_task('f', F_COUNT, out);
        latch_final.count_down();

        latch_d.wait();
        run_task('j', J_COUNT, out);
        latch_final.count_down();
        };

    std::vector<std::jthread> threads;
    threads.reserve(NT);

    threads.emplace_back(t1);
    threads.emplace_back(t2);
    threads.emplace_back(t3);
    threads.emplace_back(t4);
    threads.emplace_back(t5);

   
    latch_final.wait();

    out << "cacl end.\n";
    out.emit();

    return 0;
}