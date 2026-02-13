# smart_ptr - C++98 智能指针库

简洁、高效的 C++98/03 智能指针实现，提供 `shared_ptr`、`weak_ptr` 和 `unique_ptr`。

## 文件说明

### 核心文件
- **smart_ptr.h** - 单线程版本（推荐用于大多数场景）
- **smart_ptr_mt.h** - 线程安全版本（带原子操作，支持 GCC/Clang）
- **smart_ptr_original.h** - 原始版本备份

### 测试文件
- **test_smart_ptr.cpp** - 单元测试（用于 smart_ptr.h）
- **test_comprehensive.cpp** - 完整测试套件（7个测试）
- **test_thread_safety.cpp** - 多线程测试（需要 C++11，用于 smart_ptr_mt.h）
- **demo.cpp** - 演示程序

### 构建和测试脚本

**MSVC (单线程版本):**
- **build.bat** - 构建 demo 和单元测试
- **test_run.bat** - 运行所有测试

**GCC (单线程版本):**
- **build_gcc.bat** - 构建 demo 和单元测试
- **test_gcc.bat** - 运行所有测试

**单线程版本简化测试:**
- **test_smart_ptr.bat** - MSVC 快速测试
- **test_smart_ptr_gcc.bat** - GCC 快速测试
- **test_smart_ptr_msvc.bat** - MSVC 快速测试（备用）

**工具:**
- **clean.bat** - 清理生成的文件

## 快速开始

### 方法一：使用完整测试脚本

**GCC:**
```bash
test_gcc.bat
```

**MSVC:**
```bash
test_run.bat
```

### 方法二：使用简化测试

**GCC:**
```bash
test_smart_ptr_gcc.bat
```

**MSVC:**
```bash
test_smart_ptr.bat
```

### 方法三：手动编译和测试

GCC:
```bash
# 构建单元测试
g++ -std=c++98 -Wall -O2 -o test_smart_ptr.exe test_smart_ptr.cpp
./test_smart_ptr.exe

# 构建演示程序
g++ -std=c++98 -Wall -O2 -o demo.exe demo.cpp
./demo.exe
```

MSVC:
```bash
cl -nologo -W4 -EHsc -O2 test_smart_ptr.cpp
test_smart_ptr.exe

cl -nologo -W4 -EHsc -O2 demo.cpp
demo.exe
```

## 测试覆盖

### test_smart_ptr.cpp (单线程)
- shared_ptr 基本操作
- shared_ptr 拷贝语义
- weak_ptr 过期跟踪
- unique_ptr 基本操作
- 等等...

### test_comprehensive.cpp (单线程，7个测试)
1. shared_ptr 基本操作
2. shared_ptr 拷贝语义
3. weak_ptr 过期跟踪
4. unique_ptr 基本操作
5. reset 函数
6. swap 函数
7. 比较运算符

### test_thread_safety.cpp (多线程，需要 C++11)
- 并发拷贝操作
- 快速创建/销毁循环
- weak_ptr lock 竞争
- reset 操作
- swap 操作
- 混合操作压力测试

## 使用示例

```cpp
#include "smart_ptr.h"
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
// unique_ptr 不可拷贝，只能移动
```

## 主要特性

1. **C++98/03 兼容** - 无 C++11 特性，支持老版本编译器
2. **Safe Bool Idiom** - 防止意外的类型转换（`int x = sp;` 不再编译）
3. **RAII 设计** - 自动资源管理
4. **完整接口** - 支持 `reset()`, `swap()`, `use_count()`, `expired()` 等
5. **自定义内存管理器** - 支持 `std_mem_mgr`, `com_mem_mgr`, `array_mem_mgr`

## 编译器兼容性

| 编译器 | 版本 | 状态 |
|--------|------|------|
| MSVC   | VS2005+ | ✅ 测试通过 |
| GCC    | 4.x+ | ✅ 测试通过 |
| Clang  | 3.x+ | ✅ 应该兼容 |

## 已知问题

- 单线程版本（smart_ptr.h）不支持多线程环境
- GCC `-O2` 优化时可能产生 `use-after-free` 警告（误报，不影响运行）
- 如需线程安全，请使用 **smart_ptr_mt.h**

## 清理生成的文件

```bash
clean.bat
```

## 版本历史

### v1.1 (当前版本)
- ✅ 修复 release() 函数的 use-after-free 问题
- ✅ 添加 Safe Bool Idiom
- ✅ 移除 unique_ptr 的 virtual 析构函数
- ✅ 修复命名空间结束符分号问题
- ✅ 完整测试套件

### v1.0 (原始版本)
- 基本的 shared_ptr、weak_ptr、unique_ptr 实现

## 许可证

MIT License - 基于 oddman 的原始实现
