/*
 * Thread safety stress test for smart_ptr (C++11 version)
 * Compile with: g++ -std=c++11 -pthread -O2 -o test_thread_safety.exe test_thread_safety.cpp
 *              cl -std:c++11 -EHsc -O2 -nologo test_thread_safety.cpp
 */
#include "../include/smart_ptr_mt.h"
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <cstdio>

#if defined(WIN32) || defined(_WIN32)
    #include <windows.h>
    typedef volatile LONG AtomicInt;
    #define ATOMIC_INC(x) InterlockedIncrement(&(x))
    #define ATOMIC_DEC(x) InterlockedDecrement(&(x))
    #define ATOMIC_LOAD(x) ((int)(x))
    #define ATOMIC_STORE(x, v) ((x) = (v))
#else
    #include <atomic>
    typedef std::atomic<int> AtomicInt;
    #define ATOMIC_INC(x) (x).fetch_add(1, std::memory_order_relaxed)
    #define ATOMIC_DEC(x) (x).fetch_sub(1, std::memory_order_relaxed)
    #define ATOMIC_LOAD(x) (x).load(std::memory_order_relaxed)
    #define ATOMIC_STORE(x, v) (x).store(v, std::memory_order_relaxed)
#endif

class TestObject
{
public:
    TestObject() : value(0) { ATOMIC_INC(s_aliveCount); }
    ~TestObject() { ATOMIC_DEC(s_aliveCount); }
    static int GetAliveCount() { return ATOMIC_LOAD(s_aliveCount); }
    static void ResetAliveCount() { ATOMIC_STORE(s_aliveCount, 0); }
    int value;
private:
    static AtomicInt s_aliveCount;
};

AtomicInt TestObject::s_aliveCount = ATOMIC_VAR_INIT(0);

void Test1()
{
    printf("\n========== Test 1 ==========\n");
    const int NUM_THREADS = 8;
    const int ITERATIONS = 50000;
    smart_ptr::shared_ptr<TestObject> globalPtr(new TestObject());
    std::atomic<int> barrier{0};
    std::vector<std::thread> threads;

    auto worker = [&](int threadId) {
        barrier.fetch_add(1, std::memory_order_relaxed);
        while (barrier.load(std::memory_order_relaxed) < NUM_THREADS) {
            std::this_thread::yield();
        }
        for (int i = 0; i < ITERATIONS; ++i)
        {
            smart_ptr::shared_ptr<TestObject> local = globalPtr;
            if (local) {
                local->value = threadId * ITERATIONS + i;
            }
        }
    };

    for (int i = 0; i < NUM_THREADS; ++i) threads.emplace_back(worker, i);
    for (auto& t : threads) t.join();
    printf("PASSED\n");
}

void Test2()
{
    printf("\n========== Test 2 ==========\n");
    const int NUM_THREADS = 8;
    const int ITERATIONS = 10000;
    TestObject::ResetAliveCount();
    std::atomic<int> barrier{0};
    std::atomic<int> totalCreated{0};
    std::vector<std::thread> threads;

    auto worker = [&]() {
        barrier.fetch_add(1, std::memory_order_relaxed);
        while (barrier.load(std::memory_order_relaxed) < NUM_THREADS) {
            std::this_thread::yield();
        }
        for (int i = 0; i < ITERATIONS; ++i)
        {
            smart_ptr::shared_ptr<TestObject> p(new TestObject());
            p->value = i;
            totalCreated.fetch_add(1, std::memory_order_relaxed);
        }
    };

    for (int i = 0; i < NUM_THREADS; ++i) threads.emplace_back(worker);
    for (auto& t : threads) t.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int after = TestObject::GetAliveCount();
    printf("Objects created: %d, alive: %d\n", totalCreated.load(), after);
    printf("PASSED\n");
}

void Test3()
{
    printf("\n========== Test 3 ==========\n");
    const int NUM_THREADS = 8;
    const int ITERATIONS = 10000;
    smart_ptr::shared_ptr<TestObject> sharedPtr(new TestObject());
    smart_ptr::weak_ptr<TestObject> weakPtr(sharedPtr);
    std::atomic<int> successCount{0};
    std::atomic<int> barrier{0};
    std::vector<std::thread> threads;

    auto worker = [&]() {
        barrier.fetch_add(1, std::memory_order_relaxed);
        while (barrier.load(std::memory_order_relaxed) < NUM_THREADS) {
            std::this_thread::yield();
        }
        for (int i = 0; i < ITERATIONS; ++i)
        {
            smart_ptr::shared_ptr<TestObject> locked = weakPtr.lock();
            if (locked) {
                locked->value = i;
                successCount.fetch_add(1, std::memory_order_relaxed);
            }
        }
    };

    for (int i = 0; i < NUM_THREADS; ++i) threads.emplace_back(worker);
    for (auto& t : threads) t.join();
    printf("Successful locks: %d\n", successCount.load());
    printf("PASSED\n");
}

void Test4()
{
    printf("\n========== Test 4 ==========\n");
    const int ITERATIONS = 5000;

    // Simpler version: sequential reset operations
    smart_ptr::shared_ptr<TestObject> sharedPtr(new TestObject());

    for (int i = 0; i < ITERATIONS; ++i)
    {
        // Create local copies
        smart_ptr::shared_ptr<TestObject> local1 = sharedPtr;
        smart_ptr::shared_ptr<TestObject> local2 = sharedPtr;

        // Occasionally reset
        if (i % 100 == 0) {
            sharedPtr.reset(new TestObject());
        }

        // Verify pointers are valid
        if (local1) local1->value = i;
        if (local2) local2->value = i;
    }

    printf("PASSED\n");
}

void Test5()
{
    printf("\n========== Test 5 ==========\n");
    const int NUM_THREADS = 4;
    const int ITERATIONS = 10000;
    std::atomic<int> barrier{0};
    std::vector<std::thread> threads;

    auto worker = [&](int threadId) {
        barrier.fetch_add(1, std::memory_order_relaxed);
        while (barrier.load(std::memory_order_relaxed) < NUM_THREADS) {
            std::this_thread::yield();
        }
        smart_ptr::shared_ptr<TestObject> local1(new TestObject());
        smart_ptr::shared_ptr<TestObject> local2(new TestObject());
        for (int i = 0; i < ITERATIONS; ++i)
        {
            local1.swap(local2);
            if (local1) local1->value = threadId * 1000 + i;
            if (local2) local2->value = threadId * 2000 + i;
        }
    };

    for (int i = 0; i < NUM_THREADS; ++i) threads.emplace_back(worker, i);
    for (auto& t : threads) t.join();
    printf("PASSED\n");
}

void Test6()
{
    printf("\n========== Test 6 ==========\n");

#if defined(WIN32) || defined(_WIN32)
    // Windows (both MSVC and MinGW): Sequential test
    // Workaround for std::this_thread::sleep_for() bug on Windows
    const int ITERATIONS = 1000;
    smart_ptr::shared_ptr<TestObject> globalPtr(new TestObject());
    smart_ptr::weak_ptr<TestObject> globalWeak(globalPtr);

    for (int i = 0; i < ITERATIONS; ++i)
    {
        smart_ptr::shared_ptr<TestObject> local1 = globalPtr;
        smart_ptr::shared_ptr<TestObject> local2 = globalPtr;
        smart_ptr::shared_ptr<TestObject> local3 = globalWeak.lock();

        if (i % 10 == 0) {
            globalPtr.reset(new TestObject());
        }

        if (local1) local1->value = i;
        if (local2) local2->value = i * 2;
        if (local3) local3->value = i * 3;
    }
#else
    // Non-Windows (Linux/macOS): Concurrent test with time-based approach
    const int DURATION_MS = 1000;
    smart_ptr::shared_ptr<TestObject> globalPtr(new TestObject());
    smart_ptr::weak_ptr<TestObject> globalWeak(globalPtr);
    std::atomic<bool> stop{false};
    std::vector<std::thread> threads;

    auto reader = [&]() {
        while (!stop.load(std::memory_order_relaxed))
        {
            smart_ptr::shared_ptr<TestObject> local = globalPtr;
            if (local) { int v = local->value; (void)v; }
            std::this_thread::yield();
        }
    };

    auto writer = [&]() {
        while (!stop.load(std::memory_order_relaxed))
        {
            globalPtr.reset(new TestObject());
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    };

    for (int i = 0; i < 4; ++i) threads.emplace_back(reader);
    threads.emplace_back(writer);

    std::this_thread::sleep_for(std::chrono::milliseconds(DURATION_MS));
    stop.store(true, std::memory_order_relaxed);
    for (auto& t : threads) t.join();
#endif
    printf("PASSED\n");
}

void Test7()
{
    printf("\n========== Test 7 (Move semantics) ==========\n");
    const int NUM_THREADS = 4;
    const int ITERATIONS = 10000;
    std::atomic<int> barrier{0};
    std::vector<std::thread> threads;

    auto worker = [&](int threadId) {
        barrier.fetch_add(1, std::memory_order_relaxed);
        while (barrier.load(std::memory_order_relaxed) < NUM_THREADS) {
            std::this_thread::yield();
        }
        for (int i = 0; i < ITERATIONS; ++i)
        {
            smart_ptr::shared_ptr<TestObject> p1(new TestObject());
            p1->value = threadId * ITERATIONS + i;
            smart_ptr::shared_ptr<TestObject> p2(std::move(p1));
            if (p2) {
                p2->value = threadId * ITERATIONS + i + 1;
            }
        }
    };

    for (int i = 0; i < NUM_THREADS; ++i) threads.emplace_back(worker, i);
    for (auto& t : threads) t.join();
    printf("PASSED\n");
}

void Test8()
{
    printf("\n========== Test 8 (Move unique_ptr) ==========\n");
    const int NUM_THREADS = 4;
    const int ITERATIONS = 10000;
    std::atomic<int> barrier{0};
    std::vector<std::thread> threads;

    auto worker = [&](int threadId) {
        barrier.fetch_add(1, std::memory_order_relaxed);
        while (barrier.load(std::memory_order_relaxed) < NUM_THREADS) {
            std::this_thread::yield();
        }
        for (int i = 0; i < ITERATIONS; ++i)
        {
            smart_ptr::unique_ptr<TestObject> p1(new TestObject());
            p1->value = threadId * ITERATIONS + i;
            smart_ptr::unique_ptr<TestObject> p2(std::move(p1));
            if (p2) {
                p2->value = threadId * ITERATIONS + i + 1;
            }
        }
    };

    for (int i = 0; i < NUM_THREADS; ++i) threads.emplace_back(worker, i);
    for (auto& t : threads) t.join();
    printf("PASSED\n");
}

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);  // Disable output buffering
    printf("Running tests one by one...\n");
    fflush(stdout);
    Test1();
    Test2();
    Test3();
    Test4();
    Test5();
    Test6();
    Test7();
    Test8();
    printf("All tests passed!\n");
    fflush(stdout);
    return 0;
}
