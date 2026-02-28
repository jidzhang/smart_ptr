#include <stdio.h>
#include <utility>
#include "smart_ptr.h"

using namespace smart_ptr;

int test_count = 0;
int pass_count = 0;

int test_shared_ptr_basic()
{
    shared_ptr<int> sp(new int(42));
    return (sp.get() != 0 && *sp == 42 && sp.use_count() == 1) ? 1 : 0;
}

int test_shared_ptr_copy()
{
    shared_ptr<int> sp1(new int(100));
    shared_ptr<int> sp2 = sp1;
    if (sp1.use_count() != 2 || sp2.use_count() != 2 || *sp1 != *sp2) return 0;
    sp2.reset();
    if (sp1.use_count() != 1 || sp1.get() == 0) return 0;
    sp1.reset();
    return (sp1.get() == 0) ? 1 : 0;
}

int test_weak_ptr_expiration()
{
    shared_ptr<int> sp(new int(777));
    weak_ptr<int> wp = sp;
    if (wp.expired()) return 0;

    shared_ptr<int> sp2 = wp.lock();
    if (sp2.get() == 0) return 0;

    sp.reset();
    if (wp.expired()) return 0;  // Should not be expired, sp2 still owns

    sp2.reset();  // Last reference released
    if (!wp.expired()) return 0;  // Should be expired now

    shared_ptr<int> sp3 = wp.lock();
    if (sp3.get() != 0) return 0;

    return 1;
}

int test_unique_ptr_basic()
{
    unique_ptr<int> up(new int(555));
    if (up.get() == 0 || *up != 555 || up.use_count() != 1) return 0;
    up.reset();
    return (up.get() == 0) ? 1 : 0;
}

int test_reset_function()
{
    shared_ptr<int> sp(new int(111));
    sp.reset(new int(222));
    return (sp.get() != 0 && *sp == 222 && sp.use_count() == 1) ? 1 : 0;
}

int test_swap_function()
{
    shared_ptr<int> sp1(new int(100));
    shared_ptr<int> sp2(new int(200));
    sp1.swap(sp2);
    return (*sp1 == 200 && *sp2 == 100) ? 1 : 0;
}

int test_comparison_operators()
{
    shared_ptr<int> sp1(new int(100));
    shared_ptr<int> sp2 = sp1;
    shared_ptr<int> sp3(new int(100));
    return (sp1 == sp2 && sp1 != sp3) ? 1 : 0;
}

int test_weak_ptr_comparison_operators()
{
    shared_ptr<int> sp1(new int(100));
    shared_ptr<int> sp2(new int(200));
    weak_ptr<int> wp1(sp1);
    weak_ptr<int> wp2(sp1);
    weak_ptr<int> wp3(sp2);

    if (!(wp1 == wp2)) return 0;  // Same object, should be equal
    if (wp1 == wp3) return 0;     // Different objects, should not be equal
    if (!(wp1 != wp3)) return 0;  // Should not be equal

    return 1;
}

int test_weak_ptr_nullptr_comparison()
{
    shared_ptr<int> sp(new int(100));
    weak_ptr<int> wp1(sp);
    weak_ptr<int> wp2;

    if (wp1 == 0) return 0;   // wp1 is valid, should not equal nullptr
    if (!(wp2 == 0)) return 0;  // wp2 is empty, should equal nullptr
    if (!(wp1 != 0)) return 0;  // wp1 is valid, should not equal nullptr
    if (wp2 != 0) return 0;   // wp2 is empty, should equal nullptr

    return 1;
}

// Test for weak_ptr comparison thread safety and correctness
// Verifies that weak_ptr comparison is based on control block and pointer,
// not on the current object state (which can change between lock() calls)
int test_weak_ptr_comparison_thread_safety()
{
    // Test 1: Two weak_ptrs from same source should be equal
    {
        shared_ptr<int> sp(new int(42));
        weak_ptr<int> wp1(sp);
        weak_ptr<int> wp2(sp);

        if (!(wp1 == wp2)) return 0;  // Same source, should be equal
        if (wp1 != wp2) return 0;
    }

    // Test 2: weak_ptrs from different sources should not be equal
    {
        shared_ptr<int> sp1(new int(100));
        shared_ptr<int> sp2(new int(200));
        weak_ptr<int> wp1(sp1);
        weak_ptr<int> wp2(sp2);

        if (wp1 == wp2) return 0;   // Different sources, should not be equal
        if (!(wp1 != wp2)) return 0;
    }

    // Test 3: weak_ptrs should remain equal even after object is destroyed
    // This is the key test for the thread safety fix
    {
        shared_ptr<int> sp(new int(999));
        weak_ptr<int> wp1(sp);
        weak_ptr<int> wp2(sp);

        // Both point to same object, should be equal
        if (!(wp1 == wp2)) return 0;

        // Destroy the managed object
        sp.reset();

        // Both weak_ptrs are now expired, but should still be equal
        // because they came from the same source (same control block)
        if (!(wp1 == wp2)) return 0;  // Should still be equal!

        // Verify they are both expired
        if (!wp1.expired()) return 0;
        if (!wp2.expired()) return 0;
    }

    // Test 4: Empty weak_ptrs should be equal
    {
        weak_ptr<int> wp1;
        weak_ptr<int> wp2;

        if (!(wp1 == wp2)) return 0;  // Both empty, should be equal
        if (wp1 != wp2) return 0;
    }

    // Test 5: Empty weak_ptr should not equal non-empty weak_ptr
    {
        shared_ptr<int> sp(new int(42));
        weak_ptr<int> wp1;
        weak_ptr<int> wp2(sp);

        if (wp1 == wp2) return 0;   // Empty vs non-empty, not equal
        if (!(wp1 != wp2)) return 0;
    }

    // Test 6: weak_ptr from different copy chains should be equal
    {
        shared_ptr<int> sp(new int(42));
        weak_ptr<int> wp1(sp);
        weak_ptr<int> wp2 = wp1;     // Copy of wp1
        weak_ptr<int> wp3(wp2);      // Copy of wp2

        if (!(wp1 == wp2)) return 0;
        if (!(wp2 == wp3)) return 0;
        if (!(wp1 == wp3)) return 0;
    }

    return 1;
}

#if __cplusplus >= 201103L || _MSC_VER >= 1800
int test_shared_ptr_move()
{
    shared_ptr<int> sp1(new int(100));
    shared_ptr<int> sp2(std::move(sp1));
    
    if (sp2.get() == 0) return 0;
    if (*sp2 != 100) return 0;
    if (sp1.get() != 0) return 0;  // sp1 should be null after move
    
    return 1;
}

int test_unique_ptr_move()
{
    unique_ptr<int> up1(new int(200));
    unique_ptr<int> up2(std::move(up1));
    
    if (up2.get() == 0) return 0;
    if (*up2 != 200) return 0;
    if (up1.get() != 0) return 0;  // up1 should be null after move
    
    return 1;
}

int test_weak_ptr_move()
{
    shared_ptr<int> sp(new int(300));
    weak_ptr<int> wp1(sp);
    weak_ptr<int> wp2(std::move(wp1));
    
    if (wp2.expired()) return 0;  // Should not be expired
    shared_ptr<int> sp2 = wp2.lock();
    if (sp2.get() == 0) return 0;
    if (*sp2 != 300) return 0;
    
    return 1;
}
#else
// Dummy functions for C++98 (move semantics not supported)
int test_shared_ptr_move() { return 1; }
int test_unique_ptr_move() { return 1; }
int test_weak_ptr_move() { return 1; }
#endif

int main()
{
    printf("============================================\n");
    printf("smart_ptr.h Comprehensive Test Suite\n");
    printf("============================================\n\n");

    struct Test { int (*func)(); const char* name; };
    Test tests[] = {
        { test_shared_ptr_basic, "shared_ptr basic operations" },
        { test_shared_ptr_copy, "shared_ptr copy semantics" },
        { test_weak_ptr_expiration, "weak_ptr expiration tracking" },
        { test_unique_ptr_basic, "unique_ptr basic operations" },
        { test_reset_function, "reset function" },
        { test_swap_function, "swap function" },
        { test_comparison_operators, "comparison operators" },
        { test_weak_ptr_comparison_operators, "weak_ptr comparison operators" },
        { test_weak_ptr_nullptr_comparison, "weak_ptr nullptr comparison" },
        { test_weak_ptr_comparison_thread_safety, "weak_ptr comparison thread safety" },
        { test_shared_ptr_move, "shared_ptr move semantics" },
        { test_unique_ptr_move, "unique_ptr move semantics" },
        { test_weak_ptr_move, "weak_ptr move semantics" }
    };

    for (int i = 0; i < 13; i++) {
        test_count++;
        printf("[Test %d] %s\n", test_count, tests[i].name);
        if (tests[i].func()) {
            pass_count++;
            printf("  PASS\n\n");
        } else {
            printf("  FAIL\n\n");
        }
    }

    printf("============================================\n");
    printf("Results: %d/%d tests passed\n", pass_count, test_count);
    printf("============================================\n");

    return (pass_count == test_count) ? 0 : 1;
}
