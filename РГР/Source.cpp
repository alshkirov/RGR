#include <array>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <iostream>
#include <string>
#include <random>
#include <iomanip>
#include <locale>

#define no_of_philosophers 5

std::mutex g_lockprint;

struct fork
{
    std::mutex mutex;
};

struct table
{
    std::atomic<bool>                    ready{ false };
    std::array<fork, no_of_philosophers> forks;
};

class philosopher
{
private:
    std::string const name;
    table const& dinnertable;
    fork& left_fork;
    fork& right_fork;
    std::thread       lifethread;
    std::mt19937      rng{ std::random_device{}() };
public:
    philosopher(std::string n, table const& t, fork& l, fork& r) :
        name(n), dinnertable(t), left_fork(l), right_fork(r), lifethread(&philosopher::dine, this)
    {
    }

    ~philosopher()
    {
        lifethread.join();
    }

    void dine()
    {
        while (!dinnertable.ready);

        do
        {
            think();
            eat();
        } while (dinnertable.ready);
    }

    void print(std::string text)
    {
        std::lock_guard<std::mutex> cout_lock(g_lockprint);
        std::cout
            << std::left << std::setw(10) << std::setfill(' ')
            << name << text << std::endl;
    }

    void eat()
    {
        std::lock(left_fork.mutex, right_fork.mutex);

        std::lock_guard<std::mutex> left_lock(left_fork.mutex, std::adopt_lock);
        std::lock_guard<std::mutex> right_lock(right_fork.mutex, std::adopt_lock);

        print(" начал есть.");

        static thread_local std::uniform_int_distribution<> dist(1, 6);
        std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng) * 50));

        print(" закончил есть.");
    }

    void think()
    {
        static thread_local std::uniform_int_distribution<> wait(1, 6);
        std::this_thread::sleep_for(std::chrono::milliseconds(wait(rng) * 150));

        print(" думает ");
    }
};

void dine()
{
    std::cout << "Ужин начался!" << std::endl;

    table table;
    std::array<philosopher, no_of_philosophers> philosophers
    {
       {
          { "Аристотель", table, table.forks[0], table.forks[1] },
          { "Платон",    table, table.forks[1], table.forks[2] },
          { "Гегель", table, table.forks[2], table.forks[3] },
          { "Кант",      table, table.forks[3], table.forks[4] },
          { "Конфуций", table, table.forks[4], table.forks[0] },
       }
    };


    std::cout << "Ужин окончен!" << std::endl;
}

int main()
{
    setlocale(LC_ALL, "Russian");

    dine();

    return 0;
}