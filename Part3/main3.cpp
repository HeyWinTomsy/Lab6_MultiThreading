#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <random>
#include <chrono>

struct Call
{
    int id;
    int waitingTime;
};

std::queue<Call> calls;

std::mutex queueMutex;
std::mutex outputMutex;
std::mutex statsMutex;

std::condition_variable condition;

bool finished = false;

const int CALL_COUNT = 12;

int processedCalls = 0;
int totalWaitingTime = 0;

// Потік-виробник: створює дзвінки
void producer()
{
    std::mt19937 generator(
        std::random_device{}()
    );

    std::uniform_int_distribution<int> distribution(1, 10);

    for (int i = 1; i <= CALL_COUNT; ++i)
    {
        Call call{i, distribution(generator)};

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            calls.push(call);
        }

        {
            std::lock_guard<std::mutex> lock(outputMutex);
            std::cout << "[Producer] Call №"
                      << call.id
                      << ", waiting time: "
                      << call.waitingTime
                      << " s\n";
        }

        condition.notify_one();

        std::this_thread::sleep_for(
            std::chrono::milliseconds(100)
        );
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        finished = true;
    }

    condition.notify_all();
}

// Потік-споживач: обробляє дзвінки
void consumer(int consumerId)
{
    while (true)
    {
        Call call;

        {
            std::unique_lock<std::mutex> lock(queueMutex);

            condition.wait(lock, [] {
                return !calls.empty() || finished;
            });

            if (calls.empty() && finished)
                return;

            call = calls.front();
            calls.pop();
        }

        // Імітація обробки дзвінка
        std::this_thread::sleep_for(
            std::chrono::milliseconds(200)
        );

        {
            std::lock_guard<std::mutex> lock(outputMutex);

            std::cout << "[Consumer " << consumerId
                      << "] Processed call №"
                      << call.id
                      << ", waiting: "
                      << call.waitingTime
                      << " s\n";
        }

        {
            std::lock_guard<std::mutex> lock(statsMutex);

            ++processedCalls;
            totalWaitingTime += call.waitingTime;
        }
    }
}

int main()
{
    std::cout << "Simulation\n\n";

    std::thread producerThread(producer);

    std::vector<std::thread> consumers;

    consumers.emplace_back(consumer, 1);
    consumers.emplace_back(consumer, 2);

    producerThread.join();

    for (auto& thread : consumers)
    {
        thread.join();
    }

    std::cout << "\n--- Statistics ---\n";
    std::cout << "Calls: " << CALL_COUNT << '\n';
    std::cout << "Processed: " << processedCalls << '\n';

    if (processedCalls > 0)
    {
        double averageWaitingTime =
            static_cast<double>(totalWaitingTime) / processedCalls;

        std::cout << "Average waiting time: "
                  << averageWaitingTime << " s\n";
    }

    return 0;
}
