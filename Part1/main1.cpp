#include <iostream>
#include <thread>
#include <mutex>

const int N = 2;
const int ITERATIONS = N * 100000;

int counter = 0;
std::mutex counterMutex;

void increment()
{
    for (int i = 0; i < ITERATIONS; ++i)
    {
        std::lock_guard<std::mutex> lock(counterMutex);
        ++counter;
    }
}

int main()
{
    std::thread t1(increment);
    std::thread t2(increment);

    t1.join();
    t2.join();

    int expected = 2 * ITERATIONS;

    std::cout << "Expected value: " << expected << '\n';
    std::cout << "Actual value:   " << counter << '\n';

    return 0;
}