// Dispose-semantics regression tests (type-erased deleter).
//
// Verifies that shared_ptr always destroys the managed object as its
// original constructed type, no matter which static type the releasing
// pointer has — direct upcast construction, pointer casts, and
// weak_ptr::lock(). The hierarchies below deliberately have NO virtual
// destructors: under the pre-type-erasure design (deleter rebound to the
// releasing pointer's static type) every "base view dies last" case below
// was undefined behavior (derived destructor skipped).
//
// C++98-compatible; compiles warning-free under -Wall -Wextra -Werror.

#include <stdio.h>
#include "../include/smart_ptr.h"

using namespace smart_ptr;

static int g_dtors = 0;

struct PlainBase { int v; };
struct PlainDerived : PlainBase
{
    PlainDerived() { v = 7; }
    ~PlainDerived() { ++g_dtors; }
};

struct Member { ~Member() { ++g_dtors; } };
struct HolderBase { int v; };
struct HolderDerived : HolderBase
{
    Member m;
    HolderDerived() { v = 3; }
};

// shared_ptr<Base>(new Derived) must destroy as Derived
static int test_direct_upcast_construction()
{
    g_dtors = 0;
    {
        shared_ptr<PlainBase> b(new PlainDerived());
        if (b->v != 7) return 0;
        if (b.use_count() != 1) return 0;
    }
    return g_dtors == 1;
}

// after static/const casts, destruction order must not matter
static int test_cast_order_independence()
{
    // base view dies last
    g_dtors = 0;
    {
        shared_ptr<PlainDerived> d(new PlainDerived());
        shared_ptr<PlainBase> b = static_pointer_cast<PlainBase>(d);
        if (b.get() != d.get()) return 0;
        d.reset();
        if (g_dtors != 0) return 0;   // object must still be alive
        if (b->v != 7) return 0;
    }
    if (g_dtors != 1) return 0;

    // derived view dies last
    g_dtors = 0;
    {
        shared_ptr<PlainDerived> d(new PlainDerived());
        {
            shared_ptr<PlainBase> b = static_pointer_cast<PlainBase>(d);
            if (b->v != 7) return 0;
        }
        if (g_dtors != 0) return 0;
    }
    if (g_dtors != 1) return 0;

    // const cast chain, const view dies last
    g_dtors = 0;
    {
        shared_ptr<PlainDerived> d(new PlainDerived());
        shared_ptr<const PlainBase> c =
            const_pointer_cast<const PlainBase>(static_pointer_cast<PlainBase>(d));
        d.reset();
        if (g_dtors != 0) return 0;
        if (c->v != 7) return 0;
    }
    return g_dtors == 1;
}

// weak_ptr::lock() joins the group; last survivor still disposes the
// original constructed type
static int test_weak_lock_dispose()
{
    g_dtors = 0;
    shared_ptr<PlainDerived> d(new PlainDerived());
    weak_ptr<PlainDerived> w = d;
    shared_ptr<PlainBase> b = static_pointer_cast<PlainBase>(d);
    d.reset();
    {
        shared_ptr<PlainDerived> l = w.lock();
        if (l.get() == 0) return 0;
        if (g_dtors != 0) return 0;
        b.reset();
        if (g_dtors != 0) return 0;
    }
    if (g_dtors != 1) return 0;
    return w.expired() ? 1 : 0;
}

// the realistic failure mode: derived members with non-trivial
// destructors must be cleaned up even through a base-only view
static int test_member_cleanup_through_base()
{
    g_dtors = 0;
    {
        shared_ptr<HolderBase> h(new HolderDerived());
        if (h->v != 3) return 0;
    }
    return g_dtors == 1;
}

int main()
{
    struct Test { int (*func)(); const char* name; };
    const Test tests[] = {
        { test_direct_upcast_construction, "direct upcast construction" },
        { test_cast_order_independence, "cast order independence" },
        { test_weak_lock_dispose, "weak_ptr::lock dispose" },
        { test_member_cleanup_through_base, "member cleanup through base" }
    };

    int failures = 0;
    const int num_tests = sizeof(tests) / sizeof(tests[0]);
    printf("============================================\n");
    printf("smart_ptr Dispose Semantics (no virtual dtor)\n");
    printf("============================================\n\n");
    for (int i = 0; i < num_tests; i++) {
        printf("[Test %d] %s\n", i + 1, tests[i].name);
        if (tests[i].func()) {
            printf("  PASS\n\n");
        } else {
            printf("  FAIL\n\n");
            ++failures;
        }
    }
    printf("============================================\n");
    printf("Results: %d/%d tests passed\n", num_tests - failures, num_tests);
    printf("============================================\n");
    return failures == 0 ? 0 : 1;
}
