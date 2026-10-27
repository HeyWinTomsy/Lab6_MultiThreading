#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <future>
#include <chrono>
#include <utility>

using namespace std;

using MinMax = pair<int, int>;

// Послідовний пошук мінімуму та максимуму
MinMax findMinMaxSequential(const vector<int>& data)
{
    int minValue = data[0];
    int maxValue = data[0];

    for (int value : data)
    {
        if (value < minValue)
            minValue = value;

        if (value > maxValue)
            maxValue = value;
    }

    return {minValue, maxValue};
}

// Паралельний пошук мінімуму та максимуму
MinMax findMinMaxParallel(const vector<int>& data, int taskCount)
{
    vector<future<MinMax>> futures;

    size_t blockSize =
        (data.size() + taskCount - 1) / taskCount;

    for (int i = 0; i < taskCount; ++i)
    {
        size_t begin = i * blockSize;
        size_t end = min(begin + blockSize, data.size());

        if (begin >= data.size())
            break;

        futures.push_back(
            async(launch::async, [&data, begin, end]()
            {
                int minValue = data[begin];
                int maxValue = data[begin];

                for (size_t j = begin; j < end; ++j)
                {
                    if (data[j] < minValue)
                        minValue = data[j];

                    if (data[j] > maxValue)
                        maxValue = data[j];
                }

                return MinMax{minValue, maxValue};
            })
        );
    }

    int globalMin = data[0];
    int globalMax = data[0];

    for (auto& futureResult : futures)
    {
        MinMax result = futureResult.get();

        if (result.first < globalMin)
            globalMin = result.first;

        if (result.second > globalMax)
            globalMax = result.second;
    }

    return {globalMin, globalMax};
}

int main()
{
    const size_t N = 1'000'000;
    const int TASK_COUNT = 4;

    vector<int> data(N);

    mt19937 generator(42);
    uniform_int_distribution<int> distribution(-100000, 100000);

    for (int& value : data)
        value = distribution(generator);

    // Послідовний алгоритм
    auto startSequential = chrono::steady_clock::now();

    MinMax sequentialResult = findMinMaxSequential(data);

    auto endSequential = chrono::steady_clock::now();

    double sequentialTime =
        chrono::duration<double, milli>(
            endSequential - startSequential
        ).count();

    // Паралельний алгоритм
    auto startParallel = chrono::steady_clock::now();

    MinMax parallelResult =
        findMinMaxParallel(data, TASK_COUNT);

    auto endParallel = chrono::steady_clock::now();

    double parallelTime =
        chrono::duration<double, milli>(
            endParallel - startParallel
        ).count();

    // Виведення результатів
    cout << "Elements: " << N << '\n';
    cout << "Functions: " << TASK_COUNT << "\n\n";

    cout << "Inlie algorythm:\n";
    cout << "Min: " << sequentialResult.first << '\n';
    cout << "Max: " << sequentialResult.second << '\n';
    cout << "Time: " << sequentialTime << " ms\n\n";

    cout << "Paralel algorythm:\n";
    cout << "Min: " << parallelResult.first << '\n';
    cout << "Max: " << parallelResult.second << '\n';
    cout << "Time: " << parallelTime << " ms\n\n";

    cout << "Results match: "
         << (sequentialResult == parallelResult ? "Yes" : "No")
         << '\n';

    if (parallelTime > 0)
    {
        cout << "Accelertion: "
             << sequentialTime / parallelTime << '\n';
    }

    return 0;
}
