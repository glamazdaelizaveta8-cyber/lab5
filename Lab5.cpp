#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include <syncstream>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// Універсальне обчислення: спить задану кількість секунд, потім виводить назву
void compute(const string& name, int seconds) {
    this_thread::sleep_for(chrono::seconds(seconds));
    osyncstream(cout) << name << endl;
}

// "Повільне" обчислення: 7 секунд
void slow(const string& name) { compute(name, 7); }

// "Швидке" обчислення: 1 секунда
void quick(const string& name) { compute(name, 1); }

// Варіант 7
//   Обчислення: A1, A2, A3, A4, B1, B2, C
//   Повільні:   A2, B2
//   Залежності: B1(A1, A2), C(B1, B2)
//
// Схема (мінімальний час 9 с = A2 -> B1 -> C, 3 потоки одночасно):
//   main:     A2 (7 с) -> чекає A1 -> B1 (1 с) -> чекає B2 -> C (1 с)
//   потік 1:  B2 (7 с)
//   потік 2:  A1, A3, A4 (по 1 с)
// Запускаються лише 2 додаткові потоки.
void work() {
    auto start = chrono::steady_clock::now();

    // потік 1: B2 не залежить від інших обчислень
    future<void> fB2 = async(launch::async, [] {
        slow("B2");
        });

    // потік 2: A1 потрібен для B1, A3 і A4 ні від чого не залежать і ніхто від них не залежить
    future<void> fA = async(launch::async, [] {
        quick("A1");
        quick("A3");
        quick("A4");
        });

    // головний потік виконує критичний шлях A2 -> B1 -> C
    slow("A2");
    fA.get();      // B1 залежить від A1 (A2 виконано в цьому ж потоці)
    quick("B1");
    fB2.get();     // C залежить від B2 (B1 виконано в цьому ж потоці)
    quick("C");

    chrono::duration<double> elapsed = chrono::steady_clock::now() - start;
    osyncstream(cout) << "Час виконання: " << elapsed.count() << " с" << endl;
    osyncstream(cout) << "Work is done!" << endl;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    work();
    return 0;
}