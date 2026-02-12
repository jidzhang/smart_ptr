========================================
smart_ptr.h - C++98 智能指针库
========================================

文件清单
--------
smart_ptr.h              - 主头文件（单线程 C++98 版本）
smart_ptr_original.h     - 原始版本备份
smart_ptr_fixed.h        - 线程安全版本（带原子操作）
test_comprehensive.cpp   - 完整测试套件（7个测试）
test_smart_ptr_gcc.bat   - GCC 编译和测试脚本
test_smart_ptr.bat       - MSVC 编译和测试脚本

测试方法
--------
方法一：使用批处理文件

GCC 版本：
  test_smart_ptr_gcc.bat

MSVC 版本：
  test_smart_ptr.bat

方法二：手动编译

GCC：
  g++ -std=c++98 -Wall -O2 -o test.exe test_comprehensive.cpp
  ./test.exe

MSVC：
  cl -nologo -W4 -EHsc -utf-8 -O2 test_comprehensive.cpp
  test_comprehensive.exe

测试覆盖
--------
✓ Test 1: shared_ptr 基本操作
✓ Test 2: shared_ptr 拷贝语义
✓ Test 3: weak_ptr 过期跟踪
✓ Test 4: unique_ptr 基本操作
✓ Test 5: reset 函数
✓ Test 6: swap 函数
✓ Test 7: 比较运算符

关键特性
--------
1. C++98/03 标准（无 C++11 特性）
2. Safe Bool Idiom（防止意外的类型转换）
3. 完整的 shared_ptr、weak_ptr、unique_ptr 实现
4. 支持自定义内存管理器
5. 兼容 MSVC (VS2005+) 和 GCC/Clang

已知问题
--------
- GCC -O2 优化时可能产生 use-after-free 警告（误报）
- 单线程版本，不支持多线程环境
- 如需线程安全，请使用 smart_ptr_fixed.h

使用示例
--------
#include "smart_ptr.h"
using namespace smart_ptr;

// shared_ptr
shared_ptr<int> sp1(new int(42));
shared_ptr<int> sp2 = sp1;
sp1.reset();

// weak_ptr
weak_ptr<int> wp = sp2;
if (!wp.expired()) {
    shared_ptr<int> sp3 = wp.lock();
}

// unique_ptr
unique_ptr<int> up(new int(100));
up.reset();

版本历史
--------
v1.1 (当前版本)
  - 修复 release() 函数的 m_ptr 清空逻辑
  - 添加 Safe Bool Idiom
  - 移除 unique_ptr 的 virtual 析构函数
  - 修复命名空间结束符的分号问题
  - 修复 use-after-free 问题

v1.0 (原始版本)
  - 基本的 shared_ptr、weak_ptr、unique_ptr 实现
