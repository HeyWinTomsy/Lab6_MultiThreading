#include <iostream>
#include <thread>
#include <atomic>

const int N = 2;
const int ITERATIONS = N * 100000;

std::atomic<int> counter(0);

void increment()
{
    for (int i = 0; i < ITERATIONS; ++i)
    {
        counter++;
    }
}

int main()
{
    std::thread t1(increment);
    std::thread t2(increment);

    t1.join();
    t2.join();

    int expected = 2 * ITERATIONS;
    int actual = counter.load();

    std::cout << "Expected value: " << expected << std::endl;
    std::cout << "Actual value:   " << actual << std::endl;

    return 0;
}