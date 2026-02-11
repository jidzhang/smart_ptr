// Thread safety stress test for smart_ptr
// Compile with: cl -W3 -EHsc -nologo test_thread_safety.cpp

#include "smart_ptr_fixed.h"
#include <windows.h>
#include <stdio.h>
#include <process.h>

// Simple test object with lifecycle tracking
class TestObject {
public:
    TestObject() { InterlockedIncrement(&s_aliveCount); }
    ~TestObject() { InterlockedDecrement(&s_aliveCount); }

    static volatile LONG s_aliveCount;
    int value;
};
volatile LONG TestObject::s_aliveCount = 0;

// Global shared_ptr for threads to contend on
smart_ptr::shared_ptr<TestObject> g_sharedPtr;
const int ITERATIONS = 100000;

unsigned __stdcall ThreadFunc(void* param) {
    int id = (int)(size_t)param;
    for (int i = 0; i < ITERATIONS; ++i) {
        // Contention point: multiple threads copying from global shared_ptr
        smart_ptr::shared_ptr<TestObject> local = g_sharedPtr;
        if (local) {
            local->value = id * ITERATIONS + i;
        }
        // local goes out of scope, decrementing ref count
    }
    return 0;
}

struct RapidData {
    int iterations;
};

unsigned __stdcall RapidThreadFunc(void* param) {
    RapidData* data = (RapidData*)param;
    for (int i = 0; i < data->iterations; ++i) {
        smart_ptr::shared_ptr<TestObject> p(new TestObject());
        p->value = i;
        // p destroyed here
    }
    return 0;
}

struct WeakData {
    smart_ptr::weak_ptr<TestObject>* weakPtr;
    int iterations;
};

unsigned __stdcall WeakThreadFunc(void* param) {
    WeakData* data = (WeakData*)param;
    int successCount = 0;
    for (int i = 0; i < data->iterations; ++i) {
        smart_ptr::shared_ptr<TestObject> locked = data->weakPtr->lock();
        if (locked) {
            locked->value = i;
            ++successCount;
        }
    }
    return 0;
}

int main() {
    printf("Thread Safety Stress Test for smart_ptr\n");
    printf("========================================\n");
    printf("Iterations per thread: %d\n\n", ITERATIONS);

    // Test 1: Multiple threads reading/copying same shared_ptr
    printf("Test 1: 4 threads concurrently copying shared_ptr...\n");
    {
        g_sharedPtr.reset(new TestObject());
        LONG initialAlive = TestObject::s_aliveCount;
        printf("  Objects alive before test: %d\n", (int)initialAlive);

        HANDLE threads[4];
        for (int i = 0; i < 4; ++i) {
            threads[i] = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, (void*)(size_t)i, 0, NULL);
        }

        WaitForMultipleObjects(4, threads, TRUE, INFINITE);
        for (int i = 0; i < 4; ++i) {
            CloseHandle(threads[i]);
        }

        LONG finalAlive = TestObject::s_aliveCount;
        printf("  Objects alive after test: %d\n", (int)finalAlive);

        if (finalAlive != initialAlive) {
            printf("  FAILED: Object leaked or prematurely destroyed!\n");
            return 1;
        }
        printf("  PASSED\n\n");
    }

    // Reset global pointer, object should be destroyed
    g_sharedPtr.reset();
    if (TestObject::s_aliveCount != 0) {
        printf("FAILED: Objects not properly destroyed after reset!\n");
        return 1;
    }
    printf("Object properly destroyed after global reset.\n\n");

    // Test 2: Rapid acquire/release cycles
    printf("Test 2: Rapid shared_ptr creation/destruction in threads...\n");
    {
        const int RAPID_ITERATIONS = 50000;
        RapidData rapidData = { RAPID_ITERATIONS };

        LONG before = TestObject::s_aliveCount;
        HANDLE threads[4];
        for (int i = 0; i < 4; ++i) {
            threads[i] = (HANDLE)_beginthreadex(NULL, 0, RapidThreadFunc, &rapidData, 0, NULL);
        }

        WaitForMultipleObjects(4, threads, TRUE, INFINITE);
        for (int i = 0; i < 4; ++i) {
            CloseHandle(threads[i]);
        }

        // Give a small window for any pending destructors
        Sleep(100);

        LONG after = TestObject::s_aliveCount;
        printf("  Objects created: %d\n", 4 * RAPID_ITERATIONS);
        printf("  Objects still alive: %d\n", (int)after);

        if (after != before) {
            printf("  FAILED: Memory leak detected!\n");
            return 1;
        }
        printf("  PASSED\n\n");
    }

    // Test 3: weak_ptr lock contention
    printf("Test 3: weak_ptr lock contention...\n");
    {
        smart_ptr::shared_ptr<TestObject> sp(new TestObject());
        smart_ptr::weak_ptr<TestObject> wp(sp);
        WeakData weakData = { &wp, 10000 };

        HANDLE threads[4];
        for (int i = 0; i < 4; ++i) {
            threads[i] = (HANDLE)_beginthreadex(NULL, 0, WeakThreadFunc, &weakData, 0, NULL);
        }

        WaitForMultipleObjects(4, threads, TRUE, INFINITE);
        for (int i = 0; i < 4; ++i) {
            CloseHandle(threads[i]);
        }

        sp.reset();
        if (TestObject::s_aliveCount != 0) {
            printf("  FAILED: Object not destroyed!\n");
            return 1;
        }
        printf("  PASSED\n\n");
    }

    printf("========================================\n");
    printf("All thread safety tests PASSED!\n");
    return 0;
}
