# smart_ptr - C++98 智能指针库

简洁、高效的 C++98/03 智能指针实现，提供 `shared_ptr`、`weak_ptr` 和 `unique_ptr`。

## 📁 文件说明

### 核心文件
- **smart_ptr.h** - 单线程版本（推荐用于大多数场景）
- **smart_ptr_mt.h** - 多线程版本（带原子操作，线程安全）

### 源代码和测试文件
- **test_comprehensive.cpp** - smart_ptr.h 单元测试（7个测试，C++98）
- **test_comprehensive_mt.cpp** - smart_ptr_mt.h 单元测试（32个测试，C++98）
- **demo.cpp** - 演示程序（使用 smart_ptr_mt.h，严格 C++98）
- **test_smart_ptr.cpp** - catch2 框架完整测试（C++11，使用 smart_ptr_mt.h）
- **test_thread_safety.cpp** - 多线程压力测试（C++11，使用 smart_ptr_mt.h）

### 编译脚本

**编译脚本（3个）：**
```
build_msvc.bat    - MSVC 编译 (demo + test_smart_ptr + test_thread_safety)
build_gcc.bat      - GCC 编译
build_demo.bat     - 单独编译 demo.cpp
```

### 测试脚本

**完整测试（4个）：**
```
test_msvc.bat      - MSVC 完整测试 (demo + test_smart_ptr + test_thread_safety)
test_gcc.bat        - GCC 完整测试 (demo + test_smart_ptr + test_thread_safety)
run_100_msvc.bat   - MSVC 100次循环压力测试
run_100_gcc.sh     - GCC 100次循环压力测试
```

**单元测试脚本（4个）：**
```
test_smart_ptr_msvc.bat       - smart_ptr.h + MSVC  (7个测试)
test_smart_ptr_gcc.bat        - smart_ptr.h + GCC   (7个测试)
test_smart_ptr_mt_msvc.bat    - smart_ptr_mt.h + MSVC (32个测试)
test_smart_ptr_mt_gcc.bat     - smart_ptr_mt.h + GCC   (32个测试)
```

**工具：**
- **clean.bat** - 清理生成的文件

## 🚀 快速开始

### 1. 快速单元测试

**MSVC：**
```bash
# 测试 smart_ptr.h (单线程)
test_smart_ptr_msvc.bat

# 测试 smart_ptr_mt.h (多线程)
test_smart_ptr_mt_msvc.bat
```

**GCC：**
```bash
# 测试 smart_ptr.h (单线程)
test_smart_ptr_gcc.bat

# 测试 smart_ptr_mt.h (多线程)
test_smart_ptr_mt_gcc.bat
```

### 2. 完整功能测试 + 压力测试

**MSVC：**
```bash
test_msvc.bat         # 单次完整测试
run_100_msvc.bat      # 100次压力测试
```

**GCC：**
```bash
test_gcc.bat           # 单次完整测试
./run_100_gcc.sh      # 100次压力测试
```

### 3. 编译 demo

```bash
build_demo.bat         # MSVC
# 或
g++ -std=c++98 -Wall -O2 -o demo.exe demo.cpp  # GCC
```

## 📋 测试覆盖

### test_comprehensive.cpp (smart_ptr.h，7个测试)
1. shared_ptr 基本操作
2. shared_ptr 拷贝语义
3. weak_ptr 过期跟踪
4. unique_ptr 基本操作
5. reset 函数
6. swap 函数
7. 比较运算符

### test_comprehensive_mt.cpp (smart_ptr_mt.h，32个测试)
1-7. 基础功能（shared_ptr、weak_ptr、unique_ptr 基本操作）
8-11. 比较运算符、operator!()、跨类型比较、owner_before()
12-14. make_shared、make_unique、nullptr 比较
15-16. unique_ptr swap、weak_ptr swap
17-20. weak_ptr 扩展（默认构造、拷贝赋值、reset、use_count）
21-23. unique_ptr 扩展（默认构造、reset(new)、unique()）
24-27. shared_ptr 扩展（get()、解引用、从 weak_ptr 构造、unique()）
28-30. shared_array（基本操作、拷贝、swap）
31-32. 弱引用赋值、指针转换

### test_smart_ptr.cpp (catch2 框架)
- 完整的功能测试覆盖
- 使用 catch2 框架的测试用例

### test_thread_safety.cpp (多线程压力测试)
- 并发拷贝操作
- 快速创建/销毁循环
- weak_ptr lock 竞争
- reset/swap 混合操作压力测试

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

| 编译器 | 版本 | smart_ptr.h | smart_ptr_mt.h |
|--------|------|-------------|---------------|
| MSVC   | VS2005+ | ✅ 7/7 通过 | ✅ 32/32 通过 |
| GCC    | 4.x+   | ✅ 7/7 通过 | ✅ 32/32 通过 |
| Clang  | 3.x+   | ✅ 兼容 | ✅ 兼容 |

## 🔧 手动编译示例

### GCC
```bash
# 编译单元测试
g++ -std=c++98 -Wall -O2 -o test.exe test_comprehensive.cpp

# 编译 demo
g++ -std=c++98 -Wall -O2 -o demo.exe demo.cpp
```

### MSVC
```bash
# 编译单元测试
cl -nologo -W4 -EHsc -utf-8 -O2 test_comprehensive.cpp

# 编译 demo
cl -nologo -W4 -EHsc -utf-8 -O2 demo.cpp
```

## 🧹 清理生成的文件

```bash
clean.bat
```

## 📌 版本历史

### v1.2 (当前版本)
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

**注意：**
- 单线程版本（smart_ptr.h）适合单线程应用
- 多线程应用请使用 smart_ptr_mt.h
- 编译时请确保使用 `-utf-8` 选项（MSVC）处理 UTF-8 源文件
