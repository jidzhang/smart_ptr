// Stress test for smart_ptr_mt.h ref_count concurrent deletion
// Verifies the "extra weak ref" fix prevents double-free race in release()
//
// Race scenario (before fix):
//   Thread A (last shared_ptr): dec_ref()->0, then checks get_weak_ref_count()==0
//   Thread B (last weak_ptr):   dec_weak_ref()->0, then checks get_ref_count()==0
//   Both see (0,0), both delete counter -> double-free
//
// Fix: ref_count starts with m_weak_ref_count=1 (the "strong group" ref).
// Counter deletion is decided by a single atomic dec_weak_ref(), not by
// two separate reads.

#include "smart_ptr_mt.h"
#include <stdio.h>
#include <thread>
#include <atomic>

using namespace smart_ptr;

// --- Helpers ---

static std::atomic<int> g_ctor_count(0);
static std::atomic<int> g_dtor_count(0);

struct TrackObj {
    int val;
    TrackObj(int v) : val(v) {
        g_ctor_count.fetch_add(1, std::memory_order_relaxed);
    }
    ~TrackObj() {
        g_dtor_count.fetch_add(1, std::memory_order_relaxed);
    }
};

// Spin-barrier: all threads must arrive before any proceeds
struct SpinBarrier {
    std::atomic<int> count;
    int target;
    explicit SpinBarrier(int n) : count(0), target(n) {}
    void wait() {
        count.fetch_add(1, std::memory_order_acq_rel);
        while (count.load(std::memory_order_acquire) < target) {}
    }
};

// --- Tests ---

// Test 1: Core race — last shared_ptr and last weak_ptr destroyed concurrently
// This is the exact scenario from the bug report.
bool test_last_sp_last_wp_concurrent() {
    const int ITERS = 10000;
    g_ctor_count.store(0);
    g_dtor_count.store(0);

    for (int i = 0; i < ITERS; ++i) {
        shared_ptr<TrackObj> sp(new TrackObj(i));
        weak_ptr<TrackObj> wp = sp;

        SpinBarrier b(2);
        std::thread t1([&]() { b.wait(); sp.reset(); });
        std::thread t2([&]() { b.wait(); wp.reset(); });
        t1.join();
        t2.join();
    }

    int ctors = g_ctor_count.load();
    int dtors = g_dtor_count.load();
    if (ctors != ITERS || dtors != ITERS) {
        printf("  FAIL: ctor=%d dtor=%d expected=%d\n", ctors, dtors, ITERS);
        return false;
    }
    return true;
}

// Test 2: 3 shared_ptrs + 1 weak_ptr all destroyed concurrently
// All 4 threads hit release() at the same time.
bool test_multi_sp_one_wp() {
    const int ITERS = 5000;
    g_ctor_count.store(0);
    g_dtor_count.store(0);

    for (int i = 0; i < ITERS; ++i) {
        shared_ptr<TrackObj> sp1(new TrackObj(i));
        shared_ptr<TrackObj> sp2 = sp1;
        shared_ptr<TrackObj> sp3 = sp1;
        weak_ptr<TrackObj> wp = sp1;

        SpinBarrier b(4);
        std::thread t1([&]() { b.wait(); sp1.reset(); });
        std::thread t2([&]() { b.wait(); sp2.reset(); });
        std::thread t3([&]() { b.wait(); sp3.reset(); });
        std::thread t4([&]() { b.wait(); wp.reset(); });
        t1.join(); t2.join(); t3.join(); t4.join();
    }

    int ctors = g_ctor_count.load();
    int dtors = g_dtor_count.load();
    if (ctors != ITERS || dtors != ITERS) {
        printf("  FAIL: ctor=%d dtor=%d expected=%d\n", ctors, dtors, ITERS);
        return false;
    }
    return true;
}

// Test 3: 1 shared_ptr + 3 weak_ptrs all destroyed concurrently
// Only one thread should delete the counter (the last dec_weak_ref).
bool test_one_sp_multi_wp() {
    const int ITERS = 5000;
    g_ctor_count.store(0);
    g_dtor_count.store(0);

    for (int i = 0; i < ITERS; ++i) {
        shared_ptr<TrackObj> sp(new TrackObj(i));
        weak_ptr<TrackObj> wp1 = sp;
        weak_ptr<TrackObj> wp2 = wp1;
        weak_ptr<TrackObj> wp3 = wp1;

        SpinBarrier b(4);
        std::thread t1([&]() { b.wait(); sp.reset(); });
        std::thread t2([&]() { b.wait(); wp1.reset(); });
        std::thread t3([&]() { b.wait(); wp2.reset(); });
        std::thread t4([&]() { b.wait(); wp3.reset(); });
        t1.join(); t2.join(); t3.join(); t4.join();
    }

    int ctors = g_ctor_count.load();
    int dtors = g_dtor_count.load();
    if (ctors != ITERS || dtors != ITERS) {
        printf("  FAIL: ctor=%d dtor=%d expected=%d\n", ctors, dtors, ITERS);
        return false;
    }
    return true;
}

// Test 4: Concurrent lock() while shared_ptr is being destroyed
// Thread A destroys shared_ptr; Thread B tries to lock weak_ptr.
// If B locks before A destroys, B gets a valid shared_ptr (use_count >= 2).
// If B locks after A destroys, B gets empty. Either is valid; crash is not.
bool test_concurrent_lock_and_destroy() {
    const int ITERS = 10000;

    for (int i = 0; i < ITERS; ++i) {
        shared_ptr<TrackObj> sp(new TrackObj(i));
        weak_ptr<TrackObj> wp = sp;

        SpinBarrier b(2);
        std::thread t1([&]() { b.wait(); sp.reset(); });
        std::thread t2([&]() {
            b.wait();
            shared_ptr<TrackObj> locked = wp.lock();
        });
        t1.join();
        t2.join();
    }
    return true;
}

// Test 5: Concurrent weak_ptr copy while shared_ptr is being destroyed
// Thread A destroys shared_ptr; Thread B copies the weak_ptr.
// With the fix (extra weak ref), B's copy should always succeed
// (acquire the counter), even if A has already deallocated the object.
bool test_concurrent_wp_copy_and_sp_destroy() {
    const int ITERS = 10000;

    for (int i = 0; i < ITERS; ++i) {
        shared_ptr<TrackObj> sp(new TrackObj(i));
        weak_ptr<TrackObj> wp = sp;

        SpinBarrier b(2);
        std::thread t1([&]() { b.wait(); sp.reset(); });
        std::thread t2([&]() {
            b.wait();
            weak_ptr<TrackObj> copy = wp;
        });
        t1.join();
        t2.join();
    }
    return true;
}

int main() {
    printf("============================================\n");
    printf("smart_ptr_mt Race Condition Stress Tests\n");
    printf("============================================\n\n");

    struct TestCase {
        const char* name;
        bool (*func)();
    } tests[] = {
        { "Last shared_ptr + last weak_ptr concurrent destruction (10000)",
          test_last_sp_last_wp_concurrent },
        { "3 shared_ptrs + 1 weak_ptr concurrent destruction (5000)",
          test_multi_sp_one_wp },
        { "1 shared_ptr + 3 weak_ptrs concurrent destruction (5000)",
          test_one_sp_multi_wp },
        { "Concurrent lock() and shared_ptr destruction (10000)",
          test_concurrent_lock_and_destroy },
        { "Concurrent weak_ptr copy and shared_ptr destruction (10000)",
          test_concurrent_wp_copy_and_sp_destroy },
    };

    int passed = 0;
    int total = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < total; ++i) {
        printf("[Test %d] %s...\n", i + 1, tests[i].name);
        fflush(stdout);
        if (tests[i].func()) {
            printf("  PASS\n");
            passed++;
        } else {
            printf("  FAIL\n");
        }
    }

    printf("\n============================================\n");
    printf("Results: %d/%d tests passed\n", passed, total);
    printf("============================================\n");

    return (passed == total) ? 0 : 1;
}
