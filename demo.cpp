/*
 * smart_ptr demo program
 * Demonstrates shared_ptr, weak_ptr, unique_ptr, shared_array usage
 * Compatible with C++98/C++03 and C++14
 */

#include <stdio.h>
#include "smart_ptr_mt.h"

// Test class
class TestObject
{
public:
    TestObject() : m_id(0)
    {
        printf("TestObject %d constructed (default)\n", m_id);
    }

    TestObject(int id) : m_id(id)
    {
        printf("TestObject %d constructed\n", m_id);
    }

    ~TestObject()
    {
        printf("TestObject %d destroyed\n", m_id);
    }

    void Show() const
    {
        printf("TestObject %d says hello\n", m_id);
    }

    int GetId() const { return m_id; }

private:
    int m_id;
};

// Derived class for polymorphism test
class DerivedObject : public TestObject
{
public:
    DerivedObject(int id, int extra) : TestObject(id), m_extra(extra)
    {
        printf("DerivedObject %d (extra=%d) constructed\n", GetId(), m_extra);
    }

    ~DerivedObject()
    {
        printf("DerivedObject %d destroyed\n", GetId());
    }

    int GetExtra() const { return m_extra; }

private:
    int m_extra;
};

// Demo shared_ptr basic usage
void DemoSharedPtr()
{
    printf("\n========== shared_ptr demo ==========\n");

    // 1. Create from raw pointer
    printf("\n--- 1. Basic construction ---\n");
    smart_ptr::shared_ptr<TestObject> sp1(new TestObject(1));
    sp1->Show();

    // 2. Copy construction
    printf("\n--- 2. Copy construction ---\n");
    smart_ptr::shared_ptr<TestObject> sp2(sp1);
    printf("sp1 use_count: %d\n", sp1.use_count());
    printf("sp2 use_count: %d\n", sp2.use_count());

    // 3. Assignment
    printf("\n--- 3. Assignment ---\n");
    smart_ptr::shared_ptr<TestObject> sp3;
    sp3 = sp1;
    printf("After assignment, use_count: %d\n", sp3.use_count());

    // 4. unique check
    printf("\n--- 4. unique check ---\n");
    printf("sp1.unique(): %s\n", sp1.unique() ? "true" : "false");
    smart_ptr::shared_ptr<TestObject> sp4(new TestObject(2));
    printf("sp4.unique(): %s\n", sp4.unique() ? "true" : "false");

    // 5. reset operation
    printf("\n--- 5. reset operation ---\n");
    printf("Before reset, sp1 use_count: %d\n", sp1.use_count());
    sp1.reset();
    printf("After reset, sp1 use_count: %d\n", sp1.use_count());

    // 6. get() raw pointer
    printf("\n--- 6. get() raw pointer ---\n");
    if (sp4.get())
    {
        sp4.get()->Show();
    }

    // 7. swap operation
    printf("\n--- 7. swap operation ---\n");
    smart_ptr::shared_ptr<TestObject> sp5(new TestObject(5));
    smart_ptr::shared_ptr<TestObject> sp6(new TestObject(6));
    printf("Before swap: sp5 id=%d, sp6 id=%d\n", sp5->GetId(), sp6->GetId());
    sp5.swap(sp6);
    printf("After swap: sp5 id=%d, sp6 id=%d\n", sp5->GetId(), sp6->GetId());

    // 8. Comparison operators
    printf("\n--- 8. Comparison operators ---\n");
    smart_ptr::shared_ptr<TestObject> sp7 = sp4;
    smart_ptr::shared_ptr<TestObject> sp8(new TestObject(8));
    printf("sp4 == sp7: %s\n", (sp4 == sp7) ? "true" : "false");
    printf("sp4 != sp8: %s\n", (sp4 != sp8) ? "true" : "false");

    // 9. nullptr comparison
    printf("\n--- 9. nullptr comparison ---\n");
    smart_ptr::shared_ptr<TestObject> empty;
    printf("empty == 0: %s\n", (empty == 0) ? "true" : "false");
    printf("sp4 != 0: %s\n", (sp4 != 0) ? "true" : "false");
    if (!empty) {
        printf("!empty is true (empty pointer is falsy)\n");
    }

    printf("\n--- shared_ptr demo end ---\n");
}

// Demo weak_ptr basic usage
void DemoWeakPtr()
{
    printf("\n========== weak_ptr demo ==========\n");

    // 1. Create from shared_ptr
    printf("\n--- 1. Create from shared_ptr ---\n");
    smart_ptr::shared_ptr<TestObject> sp(new TestObject(10));
    smart_ptr::weak_ptr<TestObject> wp(sp);
    printf("shared_ptr use_count: %d\n", sp.use_count());

    // 2. expired check
    printf("\n--- 2. expired check ---\n");
    printf("wp.expired(): %s\n", wp.expired() ? "true" : "false");

    // 3. lock() get shared_ptr
    printf("\n--- 3. lock() get shared_ptr ---\n");
    smart_ptr::shared_ptr<TestObject> sp2 = wp.lock();
    if (sp2.get())
    {
        sp2->Show();
        printf("After lock, use_count: %d\n", sp2.use_count());
    }

    // 4. use_count() for weak_ptr
    printf("\n--- 4. weak_ptr use_count ---\n");
    printf("wp.use_count(): %d\n", wp.use_count());

    // 5. After shared_ptr destroyed
    printf("\n--- 5. After shared_ptr destroyed ---\n");
    sp.reset();
    sp2.reset();
    printf("After reset all shared_ptr, wp.expired(): %s\n", wp.expired() ? "true" : "false");

    // 6. Try lock expired weak_ptr
    printf("\n--- 6. Lock expired weak_ptr ---\n");
    smart_ptr::shared_ptr<TestObject> sp3 = wp.lock();
    if (!sp3.get())
    {
        printf("lock() returned empty shared_ptr as expected\n");
    }

    // 7. owner_before for ordered containers
    printf("\n--- 7. owner_before (for ordered containers) ---\n");
    smart_ptr::shared_ptr<TestObject> sp4(new TestObject(40));
    smart_ptr::shared_ptr<TestObject> sp5(new TestObject(50));
    smart_ptr::weak_ptr<TestObject> wp4(sp4);
    smart_ptr::weak_ptr<TestObject> wp5(sp5);
    bool before = wp4.owner_before(wp5);
    printf("wp4.owner_before(wp5): %s\n", before ? "true" : "false");

    printf("\n--- weak_ptr demo end ---\n");
}

// Demo unique_ptr basic usage
void DemoUniquePtr()
{
    printf("\n========== unique_ptr demo ==========\n");

    // 1. Basic construction
    printf("\n--- 1. Basic construction ---\n");
    smart_ptr::unique_ptr<TestObject> up1(new TestObject(20));
    up1->Show();

    // 2. reset operation
    printf("\n--- 2. reset operation ---\n");
    up1.reset(new TestObject(21));
    up1->Show();

    // 3. Manual reset (destroy object)
    printf("\n--- 3. Manual reset ---\n");
    up1.reset();
    printf("After reset, up1.get() is %s\n", up1.get() ? "not null" : "null");

    // 4. Reassign
    printf("\n--- 4. Reassign ---\n");
    up1.reset(new TestObject(22));
    (*up1).Show();

    // 5. release() - transfer ownership
    printf("\n--- 5. release() - transfer ownership ---\n");
    smart_ptr::unique_ptr<TestObject> up2(new TestObject(23));
    TestObject* raw = up2.release();
    printf("After release, up2.get() is %s\n", up2.get() ? "not null" : "null");
    printf("raw pointer is %s\n", raw ? "valid" : "null");
    delete raw;  // Manual delete required after release

    // 6. swap operation
    printf("\n--- 6. swap operation ---\n");
    smart_ptr::unique_ptr<TestObject> up3(new TestObject(30));
    smart_ptr::unique_ptr<TestObject> up4(new TestObject(40));
    printf("Before swap: up3 id=%d, up4 id=%d\n", up3->GetId(), up4->GetId());
    up3.swap(up4);
    printf("After swap: up3 id=%d, up4 id=%d\n", up3->GetId(), up4->GetId());

    // 7. unique() check
    printf("\n--- 7. unique() check ---\n");
    smart_ptr::unique_ptr<TestObject> up5(new TestObject(50));
    printf("up5.unique(): %s\n", up5.unique() ? "true" : "false");

    printf("\n--- unique_ptr demo end ---\n");
}

// Demo shared_array basic usage
void DemoSharedArray()
{
    printf("\n========== shared_array demo ==========\n");

    // 1. Create array
    printf("\n--- 1. Create array ---\n");
    smart_ptr::shared_array<int> arr(new int[5]);
    for (int i = 0; i < 5; ++i)
    {
        arr[i] = i * 10;
    }

    // 2. Access elements
    printf("\n--- 2. Access array elements ---\n");
    printf("Array elements: ");
    for (int i = 0; i < 5; ++i)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // 3. Copy share
    printf("\n--- 3. Array copy share ---\n");
    smart_ptr::shared_array<int> arr2(arr);
    printf("arr.use_count(): %d\n", arr.use_count());
    arr2[0] = 999;
    printf("After modify arr2[0], arr[0] = %d\n", arr[0]);

    // 4. swap operation for shared_array
    printf("\n--- 4. Array swap ---\n");
    smart_ptr::shared_array<int> arr3(new int[3]);
    arr3[0] = 100; arr3[1] = 200; arr3[2] = 300;
    smart_ptr::shared_array<int> arr4(new int[3]);
    arr4[0] = 10; arr4[1] = 20; arr4[2] = 30;
    printf("Before swap: arr3[0]=%d, arr4[0]=%d\n", arr3[0], arr4[0]);
    arr3.swap(arr4);
    printf("After swap: arr3[0]=%d, arr4[0]=%d\n", arr3[0], arr4[0]);

    printf("\n--- shared_array demo end ---\n");
}

// Demo make_shared_ptr factory function
void DemoMakeShared()
{
    printf("\n========== make_shared_ptr demo ==========\n");

    // 1. No argument construction
    printf("\n--- 1. No argument construction ---\n");
    smart_ptr::shared_ptr<TestObject> sp1 =
        smart_ptr::make_shared<TestObject>();
    sp1->Show();

    // 2. With argument construction
    printf("\n--- 2. With argument construction ---\n");
    smart_ptr::shared_ptr<TestObject> sp2 =
        smart_ptr::make_shared<TestObject>(100);
    sp2->Show();

    printf("\n--- make_shared_ptr demo end ---\n");
}

// Demo make_unique factory function
void DemoMakeUnique()
{
    printf("\n========== make_unique demo ==========\n");

    // 1. No argument construction
    printf("\n--- 1. No argument construction ---\n");
    smart_ptr::unique_ptr<TestObject> up1 =
        smart_ptr::make_unique<TestObject>();
    up1->Show();

    // 2. With argument construction
    printf("\n--- 2. With argument construction ---\n");
    smart_ptr::unique_ptr<TestObject> up2 =
        smart_ptr::make_unique<TestObject>(200);
    up2->Show();

    printf("\n--- make_unique demo end ---\n");
}

// Demo polymorphism
void DemoPolymorphism()
{
    printf("\n========== Polymorphism demo ==========\n");

    // shared_ptr<base> points to derived object
    smart_ptr::shared_ptr<TestObject> basePtr(new DerivedObject(200, 42));
    basePtr->Show();

    // Copy to another shared_ptr
    smart_ptr::shared_ptr<TestObject> anotherPtr = basePtr;
    printf("use_count after copy: %d\n", basePtr.use_count());

    printf("\n--- Polymorphism demo end ---\n");
}

int main()
{
    printf("smart_ptr demo program\n");
#if defined(_MSC_VER)
    printf("Compiler: MSVC %d\n", _MSC_VER);
#elif defined(__GNUC__)
    printf("Compiler: GCC %d\n", __GNUC__);
#elif defined(__clang__)
    printf("Compiler: Clang\n");
#else
    printf("Compiler: Unknown\n");
#endif

    DemoSharedPtr();
    DemoWeakPtr();
    DemoUniquePtr();
    DemoSharedArray();
    DemoMakeShared();
    DemoMakeUnique();
    DemoPolymorphism();

    printf("\n========== All demos finished ==========\n");
    printf("All smart pointers will auto cleanup when program exits\n");

    return 0;
}
