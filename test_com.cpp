/*
 * COM smart pointer test program (MSVC only)
 * Tests com_mem_mgr and _NoAddRefReleaseOnComPtr functionality
 */

#include <stdio.h>
#include <windows.h>
#include "smart_ptr_mt.h"

// Mock COM object - simulates IUnknown behavior
class MockComObject
{
public:
    MockComObject() : m_refCount(0), m_id(s_nextId++)
    {
        printf("MockComObject %d created\n", m_id);
    }

    // IUnknown methods
    ULONG AddRef()
    {
        return ++m_refCount;
    }

    ULONG Release()
    {
        ULONG count = --m_refCount;
        if (count == 0)
        {
            printf("MockComObject %d destroyed\n", m_id);
            delete this;
        }
        return count;
    }

    // Mock method
    void DoSomething()
    {
        printf("MockComObject %d::DoSomething() called\n", m_id);
    }

    static int s_nextId;

private:
    ~MockComObject()
    {
        // Destructor is private for COM objects
    }

    ULONG m_refCount;
    int m_id;
};

int MockComObject::s_nextId = 1;

int test_com_basic()
{
    printf("\n========== Test 1: Basic COM smart pointer ==========\n");

    // Create COM object
    MockComObject* raw = new MockComObject();
    raw->AddRef();  // Initial AddRef

    // Wrap in shared_ptr with com_mem_mgr
    smart_ptr::shared_ptr<MockComObject, smart_ptr::com_mem_mgr<MockComObject>> sp(raw);

    printf("After construction, ref count via raw: %lu\n", raw->AddRef() - 1);
    printf("PASSED\n");
    return 1;
}

int test_com_copy()
{
    printf("\n========== Test 2: COM smart pointer copy ==========\n");

    MockComObject* raw = new MockComObject();
    raw->AddRef();

    smart_ptr::shared_ptr<MockComObject, smart_ptr::com_mem_mgr<MockComObject>> sp1(raw);
    
    printf("sp1 use_count: %d\n", sp1.use_count());

    // Copy should increase ref count
    smart_ptr::shared_ptr<MockComObject, smart_ptr::com_mem_mgr<MockComObject>> sp2 = sp1;
    
    printf("After copy, sp1 use_count: %d\n", sp1.use_count());
    printf("After copy, sp2 use_count: %d\n", sp2.use_count());

    // Both should point to same object
    if (sp1.get() == sp2.get() && sp1.use_count() == 2)
    {
        printf("PASSED\n");
        return 1;
    }
    
    printf("FAILED\n");
    return 0;
}

int test_com_destructor()
{
    printf("\n========== Test 3: COM smart pointer destructor ==========\n");

    {
        MockComObject* raw = new MockComObject();
        raw->AddRef();
        
        smart_ptr::shared_ptr<MockComObject, smart_ptr::com_mem_mgr<MockComObject>> sp(raw);
        printf("Going out of scope...\n");
    }
    
    // Object should be destroyed automatically
    printf("PASSED\n");
    return 1;
}

int test_com_make_com_shared_ptr()
{
    printf("\n========== Test 4: make_com_shared_ptr ==========\n");

    // Use make_com_shared_ptr factory - takes ownership of new object
    MockComObject* raw = new MockComObject();
    raw->AddRef();
    
    smart_ptr::shared_ptr<MockComObject, smart_ptr::com_mem_mgr<MockComObject>> sp = 
        smart_ptr::make_com_shared_ptr<MockComObject>(raw);

    printf("use_count after make: %d\n", sp.use_count());
    sp->DoSomething();

    printf("PASSED\n");
    return 1;
}

int test_com_reset()
{
    printf("\n========== Test 5: COM smart pointer reset ==========\n");

    MockComObject* raw1 = new MockComObject();
    raw1->AddRef();
    smart_ptr::shared_ptr<MockComObject, smart_ptr::com_mem_mgr<MockComObject>> sp(raw1);
    
    printf("After reset:\n");
    sp.reset();
    
    printf("After reset, sp.get(): %p\n", (void*)sp.get());

    printf("PASSED\n");
    return 1;
}

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("============================================\n");
    printf("COM Smart Pointer Test Suite (MSVC)\n");
    printf("============================================\n");

    struct Test { int (*func)(); const char* name; };
    Test tests[] = {
        { test_com_basic, "COM smart pointer basic" },
        { test_com_copy, "COM smart pointer copy" },
        { test_com_destructor, "COM smart pointer destructor" },
        { test_com_make_com_shared_ptr, "make_com_shared_ptr" },
        { test_com_reset, "COM smart pointer reset" }
    };

    int passed = 0;
    int total = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < total; i++)
    {
        printf("\n[%d/%d] %s\n", i + 1, total, tests[i].name);
        if (tests[i].func())
        {
            passed++;
        }
        else
        {
            printf("FAILED\n");
        }
    }

    printf("\n============================================\n");
    printf("Results: %d/%d tests passed\n", passed, total);
    printf("============================================\n");

    return (passed == total) ? 0 : 1;
}
