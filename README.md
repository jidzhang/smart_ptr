# smart_ptr - C++98 智能指针库

简洁、高效的 C++98/03 智能指针实现，提供 `shared_ptr`、`weak_ptr` 和 `unique_ptr`。

## 📁 文件说明

### 核心文件
- **include/smart_ptr.h** - 单线程版本（零原子开销，纯单线程场景）
- **include/smart_ptr_mt.h** - 多线程版本（原子引用计数，支持跨线程共享所有权）

> **如何选择？** 见下方 [版本选择指南](#版本选择指南)。

### 源代码和测试文件
- **test_comprehensive.cpp** - smart_ptr.h 单元测试（13个测试，C++11）
- **test_comprehensive_mt.cpp** - smart_ptr_mt.h 单元测试（38个测试，C++11）
- **demo.cpp** - 演示程序（使用 smart_ptr_mt.h，严格 C++98）
- **test_smart_ptr.cpp** - catch2 框架完整测试（C++11，使用 smart_ptr_mt.h）
- **test_thread_safety.cpp** - 多线程压力测试（C++11，使用 smart_ptr_mt.h）
- **test_race_condition.cpp** - 竞态条件专项测试（C++11，使用 smart_ptr_mt.h）
- **test_com.cpp** - COM 指针测试（Windows 专属）

### 编译脚本

**MSVC（Windows）：**
```
scripts/build_demo.bat     - 单独编译 demo.cpp
```

**GCC / Linux / macOS：**
```
# 无需编译脚本，命令行直接编译（见下方 GCC 编译示例）
```

### 测试脚本

**完整测试：**
```
Windows:
  scripts/test_all.bat        - MSVC 一键运行所有测试（含 COM）
  scripts/run_100_msvc.bat    - MSVC 100次循环压力测试

Linux / macOS:
  scripts/test_all_gcc.sh     - GCC 一键运行所有测试（自动跳过 COM）
  scripts/run_100_gcc.sh      - GCC 100次循环压力测试
```

**单元测试脚本：**
```
Windows (MSVC):
  scripts/test_smart_ptr_msvc.bat      - smart_ptr.h      (13个测试)
  scripts/test_smart_ptr_mt_msvc.bat   - smart_ptr_mt.h   (38个测试)
  scripts/test_com_msvc.bat            - COM 指针测试    (5个测试)

Linux / macOS (GCC):
  scripts/test_smart_ptr_gcc.sh        - smart_ptr.h      (13个测试)
  scripts/test_smart_ptr_mt_gcc.sh     - smart_ptr_mt.h   (38个测试)
```

**工具：**
- **scripts/clean.bat** - 清理生成的文件（Windows）
- **scripts/clean.sh** - 清理生成的文件（Linux / macOS）

## 📖 版本选择指南

### `smart_ptr.h` vs `smart_ptr_mt.h`

| 维度 | `smart_ptr.h` (单线程) | `smart_ptr_mt.h` (多线程) |
|------|------------------------|---------------------------|
| **引用计数** | 普通 `int`，`++`/`--` | `volatile` + 原子操作（`Interlocked` / `__sync`） |
| **weak_ptr → shared_ptr 升级** | 非原子（直接 `inc_ref()`） | 原子 CAS（`try_inc_ref()`），防止 TOCTOU |
| **counter 释放** | `m_weak_ref_count = 0` | `m_weak_ref_count = 1`（extra weak ref 修复竞态） |
| **编译依赖** | 无平台依赖 | 依赖 Windows SDK 或 GCC `__sync` 内置 |
| **性能** | 最优（无内存屏障） | 每次拷贝/析构都有全内存屏障开销 |
| **跨线程共享** | ❌ 不安全 | ✅ 安全 |

### 什么时候用 `smart_ptr.h`（单线程）

- 纯单线程程序（MFC 桌面应用、嵌入式单线程任务）
- 性能敏感的内层循环（频繁创建/销毁临时 `shared_ptr`）
- 引用计数操作在热路径上，原子开销不可接受
- 确定对象生命周期完全在一个线程内管理

```cpp
// 示例：MFC 单线程对话框
void CMyDialog::OnButtonClick()
{
    smart_ptr::shared_ptr<Data> data = LoadData();  // 安全：全部在 UI 线程
    ProcessData(data);
    // data 析构，引用计数归零
}
```

### 什么时候用 `smart_ptr_mt.h`（多线程）

- 对象需要在多个线程间共享所有权（生产者-消费者、线程池回调）
- `weak_ptr::lock()` 可能在竞争环境下调用
- 无法预先确定对象生命周期由哪个线程结束

```cpp
// 示例：线程池任务共享配置对象
smart_ptr::shared_ptr<Config> globalConfig;

void ThreadWorker()
{
    smart_ptr::shared_ptr<Config> local = globalConfig;  // 原子 inc_ref，安全
    // 使用 local...
}  // 原子 dec_ref，如果这是最后一个引用，在这里析构 Config
```

### ⚠️ 重要：线程安全边界

`smart_ptr_mt.h` 的线程安全保证**与标准库 `std::shared_ptr` 完全一致**：

| 场景 | 是否安全 |
|------|----------|
| 多个线程持有**不同**的 `shared_ptr` 对象，指向同一控制块 | ✅ **安全**（引用计数原子化） |
| 一个线程调用 `weak_ptr::lock()`，另一个线程释放最后一个 `shared_ptr` | ✅ **安全**（`try_inc_ref` 原子 CAS） |
| 多个线程**同时读写同一个** `shared_ptr` 对象 | ❌ **不安全**（需要外部 `mutex`，标准库也一样） |
| 通过 `shared_ptr` 访问被管理对象（如 `*sp`） | ❌ **不安全**（需要 `mutex` 或 `atomic` 保护对象本身） |

> **核心原则**：`smart_ptr_mt` 解决的是"多线程共享**同一个对象的所有权**"的问题，不是"程序里有多个线程"的问题。

---

## 🚀 快速开始

### 1. 快速单元测试

**MSVC（Windows）：**
```bat
:: 测试 smart_ptr.h (单线程)
scripts\test_smart_ptr_msvc.bat

:: 测试 smart_ptr_mt.h (多线程)
scripts\test_smart_ptr_mt_msvc.bat

:: 测试 COM 指针
scripts\test_com_msvc.bat
```

**GCC / Linux / macOS：**
```bash
# 编译并运行单线程测试
g++ -std=c++11 -Wall -O2 -Iinclude -o test_comprehensive tests/test_comprehensive.cpp
./test_comprehensive

# 编译并运行多线程测试
g++ -std=c++11 -Wall -O2 -Iinclude -pthread -o test_comprehensive_mt tests/test_comprehensive_mt.cpp
./test_comprehensive_mt

# 或使用现成的脚本
scripts/test_smart_ptr_gcc.sh
scripts/test_smart_ptr_mt_gcc.sh
```

### 2. 完整功能测试 + 压力测试

**MSVC（Windows）：**
```bat
scripts\test_all.bat         :: 单次完整测试（含 COM）
scripts\run_100_msvc.bat     :: 100次循环压力测试
```

**GCC / Linux / macOS：**
```bash
scripts/test_all_gcc.sh      # 单次完整测试（自动跳过 COM）
scripts/run_100_gcc.sh       # 100次循环压力测试
```

### 3. 编译 demo

**MSVC：**
```bat
scripts\build_demo.bat
```

**GCC / Linux / macOS：**
```bash
g++ -std=c++11 -Wall -O2 -Iinclude -o demo demo.cpp
./demo
```

## 📋 测试覆盖

### test_comprehensive.cpp (smart_ptr.h，13个测试)
1. shared_ptr 基本操作
2. shared_ptr 拷贝语义
3. weak_ptr 过期跟踪
4. unique_ptr 基本操作
5. reset 函数
6. swap 函数
7. 比较运算符
8. weak_ptr 比较运算符
9. weak_ptr nullptr 比较
10. weak_ptr 比较线程安全
11. shared_ptr 移动语义
12. unique_ptr 移动语义
13. weak_ptr 移动语义

### test_comprehensive_mt.cpp (smart_ptr_mt.h，38个测试)
1-7. 基础功能（shared_ptr、weak_ptr、unique_ptr 基本操作）
8-11. 比较运算符、operator!()、跨类型比较、owner_before()
12-14. make_shared、make_unique、nullptr 比较
15-16. unique_ptr swap、weak_ptr swap
17-20. weak_ptr 扩展（默认构造、拷贝赋值、reset、use_count）
21-23. unique_ptr 扩展（默认构造、reset(new)、unique()）
24-27. shared_ptr 扩展（get()、解引用、从 weak_ptr 构造、unique()）
28-30. shared_array（基本操作、拷贝、swap）
31-32. 弱引用赋值、指针转换（static/dynamic/const）
33-34. weak_ptr 比较运算符、nullptr 比较
35. weak_ptr 比较线程安全
36-38. 移动语义（shared_ptr、unique_ptr、weak_ptr）

### test_smart_ptr.cpp (catch2 框架)
- 完整的功能测试覆盖（39 个测试用例，121 个断言）
- 使用 catch2 框架的测试用例

### test_race_condition.cpp (竞态条件专项测试)
- 最后一个 shared_ptr 和最后一个 weak_ptr 并发销毁
- 多个 shared_ptr + 一个 weak_ptr 并发销毁
- 一个 shared_ptr + 多个 weak_ptr 并发销毁
- 并发 `lock()` 和 `shared_ptr` 析构
- 并发 `weak_ptr` 拷贝和 `shared_ptr` 析构

### test_thread_safety.cpp (多线程压力测试)
- 并发拷贝操作
- 快速创建/销毁循环
- weak_ptr lock 竞争
- reset/swap 混合操作压力测试
- 并发 move 操作压力测试

## 💡 使用示例

```cpp
#include "smart_ptr.h"  // 或 "smart_ptr_mt.h"
using namespace smart_ptr;

// shared_ptr - 共享所有权
shared_ptr<int> sp1(new int(42));
shared_ptr<int> sp2 = sp1;  // 引用计数 = 2
sp1.reset();  // sp2 仍然拥有对象

// weak_ptr - 弱引用，不增加引用计数
weak_ptr<int> wp = sp2;
if (!wp.expired()) {
    shared_ptr<int> sp3 = wp.lock();  // 尝试获取 shared_ptr
}

// unique_ptr - 独占所有权
unique_ptr<int> up(new int(100));
unique_ptr.release();  // 释放所有权，返回原始指针
```

## ✨ 主要特性

### C++98/03 完全兼容
- 无 C++11 特性要求（核心库）
- 支持 VS2005 - VS2022 全版本
- 支持 GCC 4.x+ / Clang 3.x+

### 现代化的接口设计
- **Safe Bool Idiom** - `if(sp)` 可以，但 `int x = sp;` 编译错误
- **operator!()** - `if (!sp)` 支持
- **完整 STL 风格接口** - 支持 `==`、`!=`、`<`、`<=`、`>`、`>=`
- **nullptr 兼容** - 通过宏自动适配 nullptr/0
- **移动语义** - C++11+ 支持移动构造和移动赋值

### 线程安全（smart_ptr_mt.h）
- 使用原子操作（Interlocked API 或 __sync builtins）
- 完全支持多线程环境
- 跨平台支持（Windows + GCC/Clang）

### 内存管理器支持
- `std_mem_mgr<T>` - 标准 delete/delete[]
- `com_mem_mgr<T>` - COM 对象（AddRef/Release）
- `array_mem_mgr<T>` - 数组支持（delete[]）

### 工厂函数（STL 风格）
- `make_shared<T>(args...)` - 创建 shared_ptr
- `make_unique<T>(args...)` - 创建 unique_ptr

## 🛠️ 编译器兼容性

| 编译器 | 版本 | smart_ptr.h | smart_ptr_mt.h | 测试状态 |
|--------|------|-------------|---------------|----------|
| MSVC   | VS2005+ | ✅ 13/13 通过 | ✅ 38/38 通过 | Windows 全功能 |
| GCC    | 4.x+   | ✅ 13/13 通过 | ✅ 38/38 通过 | Linux/macOS 全功能（COM 测试跳过） |
| Clang  | 3.x+   | ✅ 兼容 | ✅ 兼容 | 与 GCC 相同 |

### 平台差异说明

| 功能 | Windows | Linux / macOS |
|------|---------|---------------|
| `smart_ptr.h` 单线程测试 | ✅ 13/13 | ✅ 13/13 |
| `smart_ptr_mt.h` 多线程测试 | ✅ 38/38 | ✅ 38/38 |
| 竞态条件压力测试 | ✅ 5/5 | ✅ 5/5 |
| catch2 完整测试 | ✅ 39/39 | ✅ 39/39 |
| COM 指针测试 (`test_com.cpp`) | ✅ 5/5 | ⏭️ 跳过（`windows.h` 不可用） |
| 100 次循环压力测试 | ✅ 通过 | ✅ 通过 |

## 🔧 手动编译示例

### GCC / Linux / macOS

```bash
# 单线程测试
g++ -std=c++11 -Wall -O2 -Iinclude -o test_comprehensive tests/test_comprehensive.cpp
./test_comprehensive

# 多线程测试（必须加 -pthread）
g++ -std=c++11 -Wall -O2 -Iinclude -pthread -o test_comprehensive_mt tests/test_comprehensive_mt.cpp
./test_comprehensive_mt

# 竞态条件压力测试
g++ -std=c++11 -Wall -O2 -Iinclude -pthread -o test_race tests/test_race_condition.cpp
./test_race

# catch2 完整测试
g++ -std=c++11 -Wall -O2 -Iinclude -pthread -o test_smart_ptr tests/test_smart_ptr.cpp
./test_smart_ptr

# 编译 demo
g++ -std=c++11 -Wall -O2 -Iinclude -o demo demo.cpp
./demo
```

### MSVC (Windows)
```bat
:: 编译单元测试
cl -nologo -W4 -EHsc -utf-8 -O2 -Iinclude tests\test_comprehensive.cpp

:: 编译 demo
cl -nologo -W4 -EHsc -utf-8 -O2 -Iinclude demo.cpp
```

## 🧹 清理生成的文件

**Windows：**
```bat
scripts\clean.bat
```

**Linux / macOS：**
```bash
scripts/clean.sh
```

## 📌 版本历史

### v1.4 (当前版本)
- ✅ 添加 Unix 测试脚本（`test_all_gcc.sh`、`run_100_gcc.sh`、`clean.sh`）
- ✅ 完善版本选择指南（单线程 vs 多线程）
- ✅ 竞态条件专项测试（`test_race_condition.cpp`，5 个场景）
- ✅ 非 Windows 平台自动跳过 COM 测试
- ✅ 修复 release() 函数双重释放竞态（`m_weak_ref_count=1` 起始）

### v1.3
- ✅ 添加移动语义（shared_ptr、weak_ptr、unique_ptr）
- ✅ 添加 weak_ptr 比较运算符（==、!=、nullptr 比较）
- ✅ 添加单元测试覆盖移动语义和 weak_ptr 比较
- ✅ GCC 测试改用 C++11 标准（提升测试覆盖度）
- ✅ 优化注释（保留关键线程安全说明）

### v1.2
- ✅ 修复 release() 函数的 use-after-free 问题
- ✅ 添加 Safe Bool Idiom（C++03 兼容）
- ✅ 添加 operator!() 否定运算符
- ✅ 添加跨类型比较运算符
- ✅ 添加 weak_ptr::owner_before()
- ✅ 添加 make_shared/make_unique 工厂函数
- ✅ 添加 unique_ptr::release() 方法
- ✅ 统一接口，接近 STL 风格
- ✅ 完整单元测试覆盖（39个测试用例）
- ✅ 100次循环压力测试验证

### v1.1
- 基础 shared_ptr、weak_ptr、unique_ptr 实现
- 单线程版本

## 📄 许可证

MIT License - 基于 oddman 的原始实现

---

**注意事项：**
- 单线程版本（`smart_ptr.h`）适合纯单线程应用，零原子开销
- 多线程应用请使用 `smart_ptr_mt.h`，引用计数操作是原子的
- **同一个** `shared_ptr` 对象的多线程读写需要外部 `mutex` 保护（与 `std::shared_ptr` 行为一致）
- MSVC 编译时请确保使用 `-utf-8` 选项处理 UTF-8 源文件
- COM 指针测试（`test_com.cpp`）仅在 Windows 平台可用
