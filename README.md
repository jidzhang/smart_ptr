# smart_ptr - C++98 Smart Pointer Library

A concise and efficient C++98/03 smart pointer implementation providing `shared_ptr`, `weak_ptr`, and `unique_ptr`.

## File Structure

### Core Headers
- **include/smart_ptr.h** - Single-threaded version (zero atomic overhead, pure single-threaded scenarios)
- **include/smart_ptr_mt.h** - Multi-threaded version (atomic reference counting, supports cross-thread shared ownership)

> **How to choose?** See the [Version Selection Guide](#version-selection-guide) below.

### Source and Test Files
- **test_comprehensive.cpp** - Unit tests for smart_ptr.h (13 tests, C++11)
- **test_comprehensive_mt.cpp** - Unit tests for smart_ptr_mt.h (38 tests, C++11)
- **demo.cpp** - Demo program (uses smart_ptr_mt.h, strict C++98)
- **test_smart_ptr.cpp** - Full tests using catch2 framework (C++11, uses smart_ptr_mt.h)
- **test_thread_safety.cpp** - Multi-threaded stress tests (C++11, uses smart_ptr_mt.h)
- **test_race_condition.cpp** - Race condition focused tests (C++11, uses smart_ptr_mt.h)
- **test_com.cpp** - COM pointer tests (Windows only)

### Build Scripts

**MSVC (Windows):**
```
scripts/build_demo.bat     - Compile demo.cpp
```

**GCC / Linux / macOS:**
```
# No build scripts needed; compile directly from command line (see GCC examples below)
```

### Test Scripts

**Full Test Suite:**
```
Windows:
  scripts/test_all.bat        - MSVC one-click run all tests (includes COM)
  scripts/run_100_msvc.bat    - MSVC 100-run stress test

Linux / macOS:
  scripts/test_all_gcc.sh     - GCC one-click run all tests (auto-skips COM)
  scripts/run_100_gcc.sh      - GCC 100-run stress test
```

**Unit Test Scripts:**
```
Windows (MSVC):
  scripts/test_smart_ptr_msvc.bat      - smart_ptr.h      (13 tests)
  scripts/test_smart_ptr_mt_msvc.bat   - smart_ptr_mt.h   (38 tests)
  scripts/test_com_msvc.bat            - COM pointer tests (5 tests)

Linux / macOS (GCC):
  scripts/test_smart_ptr_gcc.sh        - smart_ptr.h      (13 tests)
  scripts/test_smart_ptr_mt_gcc.sh     - smart_ptr_mt.h   (38 tests)
```

**Utilities:**
- **scripts/clean.bat** - Clean build artifacts (Windows)
- **scripts/clean.sh** - Clean build artifacts (Linux / macOS)

## Version Selection Guide

### `smart_ptr.h` vs `smart_ptr_mt.h`

| Dimension | `smart_ptr.h` (Single-threaded) | `smart_ptr_mt.h` (Multi-threaded) |
|-----------|--------------------------------|-----------------------------------|
| **Reference Count** | Plain `int`, `++`/`--` | `volatile` + atomic (`Interlocked` / `__sync`) |
| **weak_ptr to shared_ptr upgrade** | Non-atomic (direct `inc_ref()`) | Atomic CAS (`try_inc_ref()`), prevents TOCTOU |
| **Counter destruction** | `m_weak_ref_count = 0` | `m_weak_ref_count = 1` (extra weak ref fixes race) |
| **Build dependency** | None | Windows SDK or GCC `__sync` builtins |
| **Performance** | Optimal (no memory barrier) | Full memory barrier on every copy/destruct |
| **Cross-thread sharing** | Not safe | Safe |

### When to use `smart_ptr.h` (Single-threaded)

- Pure single-threaded programs (MFC desktop apps, embedded single-threaded tasks)
- Performance-sensitive inner loops (frequent creation/destruction of temporary `shared_ptr`)
- Reference counting on hot paths where atomic overhead is unacceptable
- Object lifetime known to be managed entirely within one thread

```cpp
// Example: MFC single-threaded dialog
void CMyDialog::OnButtonClick()
{
    smart_ptr::shared_ptr<Data> data = LoadData();  // Safe: all on UI thread
    ProcessData(data);
    // data destructs, ref count drops to zero
}
```

### When to use `smart_ptr_mt.h` (Multi-threaded)

- Objects need shared ownership across threads (producer-consumer, thread pool callbacks)
- `weak_ptr::lock()` may be called under contention
- Cannot predetermine which thread ends the object lifetime

```cpp
// Example: Thread pool tasks sharing a config object
smart_ptr::shared_ptr<Config> globalConfig;

void ThreadWorker()
{
    smart_ptr::shared_ptr<Config> local = globalConfig;  // Atomic inc_ref, safe
    // Use local...
}  // Atomic dec_ref; if this was the last reference, Config destructs here
```

### Important: Thread Safety Boundaries

`smart_ptr_mt.h` provides exactly the same thread safety guarantees as `std::shared_ptr`:

| Scenario | Safe? |
|----------|-------|
| Multiple threads hold **different** `shared_ptr` objects pointing to the same control block | Safe (reference count is atomic) |
| One thread calls `weak_ptr::lock()` while another releases the last `shared_ptr` | Safe (`try_inc_ref` atomic CAS) |
| Multiple threads **concurrently read/write the same** `shared_ptr` object | **Not safe** (needs external `mutex`; same as standard library) |
| Accessing the managed object through `shared_ptr` (e.g. `*sp`) | **Not safe** (needs `mutex` or `atomic` on the object itself) |

> **Core principle**: `smart_ptr_mt` solves "multiple threads sharing **ownership of the same object**", not "the program has multiple threads".

---

## Quick Start

### 1. Quick Unit Tests

**MSVC (Windows):**
```bat
:: Test smart_ptr.h (single-threaded)
scripts\test_smart_ptr_msvc.bat

:: Test smart_ptr_mt.h (multi-threaded)
scripts\test_smart_ptr_mt_msvc.bat

:: Test COM pointers
scripts\test_com_msvc.bat
```

**GCC / Linux / macOS:**
```bash
# Compile and run single-threaded tests
g++ -std=c++11 -Wall -O2 -Iinclude -o test_comprehensive tests/test_comprehensive.cpp
./test_comprehensive

# Compile and run multi-threaded tests
g++ -std=c++11 -Wall -O2 -Iinclude -pthread -o test_comprehensive_mt tests/test_comprehensive_mt.cpp
./test_comprehensive_mt

# Or use the ready-made scripts
scripts/test_smart_ptr_gcc.sh
scripts/test_smart_ptr_mt_gcc.sh
```

### 2. Full Functional Test + Stress Test

**MSVC (Windows):**
```bat
scripts\test_all.bat         :: Single full test run (includes COM)
scripts\run_100_msvc.bat     :: 100-run stress test
```

**GCC / Linux / macOS:**
```bash
scripts/test_all_gcc.sh      # Single full test run (auto-skips COM)
scripts/run_100_gcc.sh       # 100-run stress test
```

### 3. Compile Demo

**MSVC:**
```bat
scripts\build_demo.bat
```

**GCC / Linux / macOS:**
```bash
g++ -std=c++11 -Wall -O2 -Iinclude -o demo demo.cpp
./demo
```

## Test Coverage

### test_comprehensive.cpp (smart_ptr.h, 13 tests)
1. shared_ptr basic operations
2. shared_ptr copy semantics
3. weak_ptr expiration tracking
4. unique_ptr basic operations
5. reset function
6. swap function
7. Comparison operators
8. weak_ptr comparison operators
9. weak_ptr nullptr comparison
10. weak_ptr comparison thread safety
11. shared_ptr move semantics
12. unique_ptr move semantics
13. weak_ptr move semantics

### test_comprehensive_mt.cpp (smart_ptr_mt.h, 38 tests)
1-7. Basic functions (shared_ptr, weak_ptr, unique_ptr basic operations)
8-11. Comparison operators, operator!(), cross-type comparison, owner_before()
12-14. make_shared, make_unique, nullptr comparison
15-16. unique_ptr swap, weak_ptr swap
17-20. weak_ptr extensions (default construct, copy assignment, reset, use_count)
21-23. unique_ptr extensions (default construct, reset(new), unique())
24-27. shared_ptr extensions (get(), dereference, construct from weak_ptr, unique())
28-30. shared_array (basic operations, copy, swap)
31-32. Weak reference assignment, pointer casts (static/dynamic/const)
33-34. weak_ptr comparison operators, nullptr comparison
35. weak_ptr comparison thread safety
36-38. Move semantics (shared_ptr, unique_ptr, weak_ptr)

### test_smart_ptr.cpp (catch2 framework)
- Full functional test coverage (39 test cases, 121 assertions)
- Tests using the catch2 framework

### test_race_condition.cpp (Race condition focused tests)
- Last shared_ptr and last weak_ptr destroyed concurrently
- Multiple shared_ptrs + one weak_ptr destroyed concurrently
- One shared_ptr + multiple weak_ptrs destroyed concurrently
- Concurrent `lock()` and `shared_ptr` destruction
- Concurrent `weak_ptr` copy and `shared_ptr` destruction

### test_thread_safety.cpp (Multi-threaded stress tests)
- Concurrent copy operations
- Fast create/destroy loops
- weak_ptr lock contention
- reset/swap mixed operation stress test
- Concurrent move operation stress test

## Usage Examples

```cpp
#include "smart_ptr.h"  // or "smart_ptr_mt.h"
using namespace smart_ptr;

// shared_ptr - shared ownership
shared_ptr<int> sp1(new int(42));
shared_ptr<int> sp2 = sp1;  // ref count = 2
sp1.reset();  // sp2 still owns the object

// weak_ptr - weak reference, does not increase ref count
weak_ptr<int> wp = sp2;
if (!wp.expired()) {
    shared_ptr<int> sp3 = wp.lock();  // Try to acquire shared_ptr
}

// unique_ptr - exclusive ownership
unique_ptr<int> up(new int(100));
unique_ptr.release();  // Release ownership, return raw pointer
```

## Key Features

### C++98/03 Fully Compatible
- No C++11 features required (core library)
- Supports VS2005 - VS2022 all versions
- Supports GCC 4.x+ / Clang 3.x+

### Modern Interface Design
- **Safe Bool Idiom** - `if(sp)` works, but `int x = sp;` is a compile error
- **operator!()** - `if (!sp)` supported
- **Full STL-style interface** - Supports `==`, `!=`, `<`, `<=`, `>`, `>=`
- **nullptr compatible** - Auto-adapts nullptr/0 via macros
- **Move semantics** - Move constructor and move assignment for C++11+

### Thread Safety (smart_ptr_mt.h)
- Atomic operations (Interlocked API or __sync builtins)
- Full multi-threaded environment support
- Cross-platform (Windows + GCC/Clang)

### Memory Manager Support
- `std_mem_mgr<T>` - Standard delete/delete[]
- `com_mem_mgr<T>` - COM objects (AddRef/Release)
- `array_mem_mgr<T>` - Array support (delete[])

### Factory Functions (STL style)
- `make_shared<T>(args...)` - Create shared_ptr
- `make_unique<T>(args...)` - Create unique_ptr

## Compiler Compatibility

| Compiler | Version | smart_ptr.h | smart_ptr_mt.h | Test Status |
|----------|---------|-------------|---------------|-------------|
| MSVC   | VS2005+ | 13/13 pass | 38/38 pass | Windows full feature |
| GCC    | 4.x+   | 13/13 pass | 38/38 pass | Linux/macOS full feature (COM skipped) |
| Clang  | 3.x+   | Compatible | Compatible | Same as GCC |

### Platform Differences

| Feature | Windows | Linux / macOS |
|---------|---------|---------------|
| `smart_ptr.h` single-threaded tests | 13/13 | 13/13 |
| `smart_ptr_mt.h` multi-threaded tests | 38/38 | 38/38 |
| Race condition stress tests | 5/5 | 5/5 |
| catch2 full tests | 39/39 | 39/39 |
| COM pointer tests (`test_com.cpp`) | 5/5 | Skipped (`windows.h` unavailable) |
| 100-run stress tests | Pass | Pass |

## Manual Compilation Examples

### GCC / Linux / macOS

```bash
# Single-threaded tests
g++ -std=c++11 -Wall -O2 -Iinclude -o test_comprehensive tests/test_comprehensive.cpp
./test_comprehensive

# Multi-threaded tests (must add -pthread)
g++ -std=c++11 -Wall -O2 -Iinclude -pthread -o test_comprehensive_mt tests/test_comprehensive_mt.cpp
./test_comprehensive_mt

# Race condition stress tests
g++ -std=c++11 -Wall -O2 -Iinclude -pthread -o test_race tests/test_race_condition.cpp
./test_race

# catch2 full tests
g++ -std=c++11 -Wall -O2 -Iinclude -pthread -o test_smart_ptr tests/test_smart_ptr.cpp
./test_smart_ptr

# Compile demo
g++ -std=c++11 -Wall -O2 -Iinclude -o demo demo.cpp
./demo
```

### MSVC (Windows)
```bat
:: Compile unit tests
cl -nologo -W4 -EHsc -utf-8 -O2 -Iinclude tests\test_comprehensive.cpp

:: Compile demo
cl -nologo -W4 -EHsc -utf-8 -O2 -Iinclude demo.cpp
```

## Clean Build Artifacts

**Windows:**
```bat
scripts\clean.bat
```

**Linux / macOS:**
```bash
scripts/clean.sh
```

## Version History

### v1.4 (Current)
- Add Unix test scripts (`test_all_gcc.sh`, `run_100_gcc.sh`, `clean.sh`)
- Complete version selection guide (single-threaded vs multi-threaded)
- Race condition focused tests (`test_race_condition.cpp`, 5 scenarios)
- Auto-skip COM tests on non-Windows platforms
- Fix release() double-free race (`m_weak_ref_count=1` initial value)

### v1.3
- Add move semantics (shared_ptr, weak_ptr, unique_ptr)
- Add weak_ptr comparison operators (==, !=, nullptr comparison)
- Add unit tests covering move semantics and weak_ptr comparison
- GCC tests use C++11 standard (improved coverage)
- Refined comments (kept critical thread safety notes)

### v1.2
- Fix release() use-after-free issue
- Add Safe Bool Idiom (C++03 compatible)
- Add operator!() negation operator
- Add cross-type comparison operators
- Add weak_ptr::owner_before()
- Add make_shared/make_unique factory functions
- Add unique_ptr::release() method
- Unified interface, close to STL style
- Complete unit test coverage (39 test cases)
- 100-run stress test validation

### v1.1
- Basic shared_ptr, weak_ptr, unique_ptr implementation
- Single-threaded version

## License

MIT License - Based on oddman's original implementation

---

**Notes:**
- Single-threaded version (`smart_ptr.h`) is suitable for pure single-threaded apps with zero atomic overhead
- For multi-threaded apps use `smart_ptr_mt.h`; reference counting operations are atomic
- Concurrent read/write on the **same** `shared_ptr` object requires an external `mutex` (same behavior as `std::shared_ptr`)
- When compiling with MSVC, use `-utf-8` to handle UTF-8 source files
- COM pointer tests (`test_com.cpp`) are only available on Windows
