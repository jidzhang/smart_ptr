/*
 * smart_ptr full test program - using catch2 framework
 * Requires C++11 (for catch2)
 */

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "smart_ptr_mt.h"
#include <string>

// Test helper class
class InstanceCounter
{
public:
    static int instanceCount;
    static int constructorCount;
    static int destructorCount;

    static void Reset()
    {
        instanceCount = 0;
        constructorCount = 0;
        destructorCount = 0;
    }

    InstanceCounter() { ++instanceCount; ++constructorCount; }
    ~InstanceCounter() { --instanceCount; ++destructorCount; }

private:
    InstanceCounter(const InstanceCounter&);
    InstanceCounter& operator=(const InstanceCounter&);
};

int InstanceCounter::instanceCount = 0;
int InstanceCounter::constructorCount = 0;
int InstanceCounter::destructorCount = 0;

class TestClass
{
public:
    TestClass() : m_value(0) {}
    TestClass(int value) : m_value(value) {}
    virtual ~TestClass() {}

    int GetValue() const { return m_value; }
    void SetValue(int value) { m_value = value; }

private:
    int m_value;
};

class DerivedClass : public TestClass
{
public:
    DerivedClass(int value, const char* name)
        : TestClass(value), m_name(name) {}

    const char* GetName() const { return m_name.c_str(); }

private:
    std::string m_name;
};

// ==================== shared_ptr tests ====================

TEST_CASE("shared_ptr - default construction", "[shared_ptr]")
{
    smart_ptr::shared_ptr<TestClass> sp;
    REQUIRE(sp.get() == 0);
    REQUIRE(sp.use_count() == 0);
    REQUIRE(sp.unique() == true);
}

TEST_CASE("shared_ptr - construction from pointer", "[shared_ptr]")
{
    InstanceCounter::Reset();
    {
        smart_ptr::shared_ptr<InstanceCounter> sp(new InstanceCounter);
        REQUIRE(sp.get() != 0);
        REQUIRE(sp.use_count() == 1);
        REQUIRE(sp.unique() == true);
        REQUIRE(InstanceCounter::instanceCount == 1);
    }
    REQUIRE(InstanceCounter::instanceCount == 0);
    REQUIRE(InstanceCounter::destructorCount == 1);
}

TEST_CASE("shared_ptr - copy construction", "[shared_ptr]")
{
    smart_ptr::shared_ptr<TestClass> sp1(new TestClass(42));
    smart_ptr::shared_ptr<TestClass> sp2(sp1);

    REQUIRE(sp1.get() == sp2.get());
    REQUIRE(sp1.use_count() == 2);
    REQUIRE(sp2.use_count() == 2);
    REQUIRE(!sp1.unique());
    REQUIRE(!sp2.unique());
}

TEST_CASE("shared_ptr - copy assignment", "[shared_ptr]")
{
    smart_ptr::shared_ptr<TestClass> sp1(new TestClass(42));
    smart_ptr::shared_ptr<TestClass> sp2;

    sp2 = sp1;

    REQUIRE(sp1.get() == sp2.get());
    REQUIRE(sp1.use_count() == 2);
    REQUIRE(sp2.use_count() == 2);
}

TEST_CASE("shared_ptr - self assignment", "[shared_ptr]")
{
    smart_ptr::shared_ptr<TestClass> sp(new TestClass(42));
    sp = sp;

    REQUIRE(sp.get() != 0);
    REQUIRE(sp.use_count() == 1);
    REQUIRE(sp->GetValue() == 42);
}

TEST_CASE("shared_ptr - reset to null", "[shared_ptr]")
{
    InstanceCounter::Reset();
    smart_ptr::shared_ptr<InstanceCounter> sp(new InstanceCounter);
    REQUIRE(InstanceCounter::instanceCount == 1);

    sp.reset();
    REQUIRE(sp.get() == 0);
    REQUIRE(sp.use_count() == 0);
    REQUIRE(InstanceCounter::instanceCount == 0);
}

TEST_CASE("shared_ptr - reset to new pointer", "[shared_ptr]")
{
    TestClass* p1 = new TestClass(1);
    TestClass* p2 = new TestClass(2);

    smart_ptr::shared_ptr<TestClass> sp(p1);
    REQUIRE(sp->GetValue() == 1);

    sp.reset(p2);
    REQUIRE(sp->GetValue() == 2);
    REQUIRE(sp.use_count() == 1);
}

TEST_CASE("shared_ptr - swap", "[shared_ptr]")
{
    TestClass* p1 = new TestClass(1);
    TestClass* p2 = new TestClass(2);

    smart_ptr::shared_ptr<TestClass> sp1(p1);
    smart_ptr::shared_ptr<TestClass> sp2(p2);

    sp1.swap(sp2);

    REQUIRE(sp1->GetValue() == 2);
    REQUIRE(sp2->GetValue() == 1);
}

TEST_CASE("shared_ptr - operators", "[shared_ptr]")
{
    smart_ptr::shared_ptr<TestClass> sp(new TestClass(100));

    REQUIRE((*sp).GetValue() == 100);
    REQUIRE(sp->GetValue() == 100);

    sp->SetValue(200);
    REQUIRE(sp->GetValue() == 200);
}

TEST_CASE("shared_ptr - polymorphism", "[shared_ptr]")
{
    smart_ptr::shared_ptr<TestClass> sp(new DerivedClass(42, "test"));

    REQUIRE(sp->GetValue() == 42);
}

// ==================== weak_ptr tests ====================

TEST_CASE("weak_ptr - default construction", "[weak_ptr]")
{
    smart_ptr::weak_ptr<TestClass> wp;
    REQUIRE(wp.expired() == true);
}

TEST_CASE("weak_ptr - construction from shared_ptr", "[weak_ptr]")
{
    smart_ptr::shared_ptr<TestClass> sp(new TestClass(42));
    smart_ptr::weak_ptr<TestClass> wp(sp);

    REQUIRE(!wp.expired());
    REQUIRE(sp.use_count() == 1);
}

TEST_CASE("weak_ptr - lock when valid", "[weak_ptr]")
{
    smart_ptr::shared_ptr<TestClass> sp(new TestClass(42));
    smart_ptr::weak_ptr<TestClass> wp(sp);

    smart_ptr::shared_ptr<TestClass> sp2 = wp.lock();
    REQUIRE(sp2.get() != 0);
    REQUIRE(sp2->GetValue() == 42);
    REQUIRE(sp.use_count() == 2);
}

TEST_CASE("weak_ptr - lock when expired", "[weak_ptr]")
{
    smart_ptr::weak_ptr<TestClass> wp;
    {
        smart_ptr::shared_ptr<TestClass> sp(new TestClass(42));
        wp = sp;
        REQUIRE(!wp.expired());
    }
    REQUIRE(wp.expired());

    smart_ptr::shared_ptr<TestClass> sp2 = wp.lock();
    REQUIRE(sp2.get() == 0);
}

TEST_CASE("weak_ptr - copy construction", "[weak_ptr]")
{
    smart_ptr::shared_ptr<TestClass> sp(new TestClass(42));
    smart_ptr::weak_ptr<TestClass> wp1(sp);
    smart_ptr::weak_ptr<TestClass> wp2(wp1);

    smart_ptr::shared_ptr<TestClass> sp1 = wp1.lock();
    smart_ptr::shared_ptr<TestClass> sp2 = wp2.lock();

    REQUIRE(sp1.get() != 0);
    REQUIRE(sp2.get() != 0);
    REQUIRE(sp1.get() == sp2.get());
}

TEST_CASE("weak_ptr - copy assignment", "[weak_ptr]")
{
    smart_ptr::shared_ptr<TestClass> sp(new TestClass(42));
    smart_ptr::weak_ptr<TestClass> wp1(sp);
    smart_ptr::weak_ptr<TestClass> wp2;

    wp2 = wp1;

    smart_ptr::shared_ptr<TestClass> sp2 = wp2.lock();
    REQUIRE(sp2.get() != 0);
    REQUIRE(sp2->GetValue() == 42);
}

TEST_CASE("weak_ptr - assignment from shared_ptr", "[weak_ptr]")
{
    smart_ptr::shared_ptr<TestClass> sp(new TestClass(42));
    smart_ptr::weak_ptr<TestClass> wp;

    wp = sp;

    REQUIRE(!wp.expired());
    smart_ptr::shared_ptr<TestClass> sp2 = wp.lock();
    REQUIRE(sp2->GetValue() == 42);
}

// ==================== unique_ptr tests ====================

TEST_CASE("unique_ptr - default construction", "[unique_ptr]")
{
    smart_ptr::unique_ptr<TestClass> up;
    REQUIRE(up.get() == 0);
}

TEST_CASE("unique_ptr - construction from pointer", "[unique_ptr]")
{
    InstanceCounter::Reset();
    {
        smart_ptr::unique_ptr<InstanceCounter> up(new InstanceCounter);
        REQUIRE(up.get() != 0);
        REQUIRE(InstanceCounter::instanceCount == 1);
    }
    REQUIRE(InstanceCounter::instanceCount == 0);
    REQUIRE(InstanceCounter::destructorCount == 1);
}

TEST_CASE("unique_ptr - operators", "[unique_ptr]")
{
    smart_ptr::unique_ptr<TestClass> up(new TestClass(42));

    REQUIRE((*up).GetValue() == 42);
    REQUIRE(up->GetValue() == 42);
}

TEST_CASE("unique_ptr - reset", "[unique_ptr]")
{
    InstanceCounter::Reset();
    smart_ptr::unique_ptr<InstanceCounter> up(new InstanceCounter);
    REQUIRE(InstanceCounter::instanceCount == 1);

    up.reset();
    REQUIRE(up.get() == 0);
    REQUIRE(InstanceCounter::instanceCount == 0);
}

TEST_CASE("unique_ptr - reset with new pointer", "[unique_ptr]")
{
    TestClass* p1 = new TestClass(1);
    TestClass* p2 = new TestClass(2);

    smart_ptr::unique_ptr<TestClass> up(p1);
    REQUIRE(up->GetValue() == 1);

    up.reset(p2);
    REQUIRE(up->GetValue() == 2);
}

TEST_CASE("unique_ptr - swap", "[unique_ptr]")
{
    TestClass* p1 = new TestClass(1);
    TestClass* p2 = new TestClass(2);

    smart_ptr::unique_ptr<TestClass> up1(p1);
    smart_ptr::unique_ptr<TestClass> up2(p2);

    up1.swap(up2);

    REQUIRE(up1->GetValue() == 2);
    REQUIRE(up2->GetValue() == 1);
}

TEST_CASE("unique_ptr - use_count and unique", "[unique_ptr]")
{
    smart_ptr::unique_ptr<TestClass> up(new TestClass(42));

    REQUIRE(up.use_count() == 1);
    REQUIRE(up.unique() == true);
}

// ==================== shared_array tests ====================

TEST_CASE("shared_array - default construction", "[shared_array]")
{
    smart_ptr::shared_array<int> sa;
    REQUIRE(sa.get() == 0);
}

TEST_CASE("shared_array - construction from pointer", "[shared_array]")
{
    int* arr = new int[5];
    for (int i = 0; i < 5; ++i) arr[i] = i;

    smart_ptr::shared_array<int> sa(arr);
    REQUIRE(sa.get() != 0);

    for (int i = 0; i < 5; ++i)
    {
        REQUIRE(sa[i] == i);
    }
}

TEST_CASE("shared_array - copy construction shares ownership", "[shared_array]")
{
    int* arr = new int[3];
    arr[0] = 10; arr[1] = 20; arr[2] = 30;

    smart_ptr::shared_array<int> sa1(arr);
    smart_ptr::shared_array<int> sa2(sa1);

    REQUIRE(sa1.get() == sa2.get());
    REQUIRE(sa1.use_count() == 2);

    sa2[0] = 999;
    REQUIRE(sa1[0] == 999);
}

TEST_CASE("shared_array - const operator[]", "[shared_array]")
{
    int* arr = new int[3];
    arr[0] = 10; arr[1] = 20; arr[2] = 30;

    const smart_ptr::shared_array<int> sa(arr);
    REQUIRE(sa[0] == 10);
    REQUIRE(sa[1] == 20);
}

// ==================== make_shared_ptr tests ====================

TEST_CASE("make_shared_ptr - no arguments", "[make_shared]")
{
    smart_ptr::shared_ptr<TestClass> sp =
        smart_ptr::make_shared<TestClass>();

    REQUIRE(sp.get() != 0);
    REQUIRE(sp.use_count() == 1);
}

TEST_CASE("make_shared_ptr - one argument", "[make_shared]")
{
    smart_ptr::shared_ptr<TestClass> sp =
        smart_ptr::make_shared<TestClass>(42);

    REQUIRE(sp->GetValue() == 42);
}

TEST_CASE("make_shared_ptr - multiple arguments", "[make_shared]")
{
    smart_ptr::shared_ptr<DerivedClass> sp =
        smart_ptr::make_shared<DerivedClass>(42, "hello");

    REQUIRE(sp->GetValue() == 42);
    REQUIRE(std::string(sp->GetName()) == "hello");
}

// ==================== edge cases tests ====================

TEST_CASE("shared_ptr - null pointer operations", "[edge]")
{
    smart_ptr::shared_ptr<TestClass> sp;

    REQUIRE(sp.use_count() == 0);
    REQUIRE(sp.unique() == true);
    REQUIRE(sp.get() == 0);
}

TEST_CASE("shared_ptr - reference counting", "[edge]")
{
    InstanceCounter::Reset();
    {
        smart_ptr::shared_ptr<InstanceCounter> sp1(new InstanceCounter);
        smart_ptr::shared_ptr<InstanceCounter> sp2 = sp1;
        smart_ptr::shared_ptr<InstanceCounter> sp3;
        sp3 = sp1;

        REQUIRE(InstanceCounter::instanceCount == 1);
        REQUIRE(sp1.use_count() == 3);
    }
    REQUIRE(InstanceCounter::instanceCount == 0);
    REQUIRE(InstanceCounter::destructorCount == 1);
}

TEST_CASE("weak_ptr - multiple weak pointers", "[edge]")
{
    smart_ptr::shared_ptr<TestClass> sp(new TestClass(42));
    smart_ptr::weak_ptr<TestClass> wp1(sp);
    smart_ptr::weak_ptr<TestClass> wp2(sp);
    smart_ptr::weak_ptr<TestClass> wp3;
    wp3 = sp;

    REQUIRE(!wp1.expired());
    REQUIRE(!wp2.expired());
    REQUIRE(!wp3.expired());

    sp.reset();

    REQUIRE(wp1.expired());
    REQUIRE(wp2.expired());
    REQUIRE(wp3.expired());
}
