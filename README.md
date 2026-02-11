# smart_ptr 智能指针库

一个兼容 C++98/C++03 的引用计数智能指针实现，同时支持 C++14。

## 文件清单

### 核心文件（使用这些）
| 文件 | 说明 |
|------|------|
| `smart_ptr_fixed.h` | **修正后的智能指针头文件（主文件）** |
| `demo.cpp` | 演示程序源代码 |
| `test_smart_ptr.cpp` | 单元测试源代码 |
| `build.bat` | 构建脚本（生成 .exe 和 .obj） |
| `test_run.bat` | 一键构建并运行测试 |

### 参考文件
| 文件 | 说明 |
|------|------|
| `smart_ptr.h` | 原始头文件（供对比参考，不使用） |
| `catch.hpp` | catch2 测试框架（单文件版） |
| `catch_with_main.hpp` | catch2 预配置头文件 |
| `catch_with_runner.hpp` | catch2 预配置头文件 |

### 生成文件（运行 build.bat 后生成）
| 文件 | 说明 |
|------|------|
| `demo.exe` | 演示程序可执行文件 |
| `test_smart_ptr.exe` | 单元测试可执行文件 |
| `*.obj` | 编译中间文件 |

## 主要修正内容

原 `smart_ptr.h` 存在的问题及修正：

### 1. `make_com_shared_ptr` 函数（第 613-616 行）
**问题**：使用了 C++11 的 `template` 关键字语法，且实现逻辑有误
```cpp
// 原代码（有问题）
template <typename T>
shared_ptr<T, com_mem_mgr<T> > make_com_shared_ptr(const T* rawPtr)
{
    return make_shared_ptr<T, com_mem_mgr<T> >::template generate<T*>(const_cast<T*&>(rawPtr));
}
```

**修正后**：
```cpp
template <typename T>
shared_ptr<T, com_mem_mgr<T> > make_com_shared_ptr(T* rawPtr)
{
    if (rawPtr)
        rawPtr->AddRef();
    return shared_ptr<T, com_mem_mgr<T> >(rawPtr);
}
```

### 2. `shared_ptr` 模板构造函数（第 318-321 行）
**问题**：当从派生类指针构造基类 `shared_ptr` 时，基类初始化列表错误
```cpp
// 原代码（有问题）
template<class Q>
explicit shared_ptr(Q* p) : base_ptr<Q, true, std_mem_mgr<Q> >(p)
```

**修正后**：
```cpp
template<class Q>
explicit shared_ptr(Q* p) : baseClass(static_cast<T*>(p))
{
    if (p)
    {
        this->m_counter = new ref_count;
    }
}
```

### 3. 编码问题（第 519 行）
**问题**：有乱码注释（`//��ֹ����`）

**修正后**：使用正确的注释 `// 禁止拷贝`

## 使用方法

### 1. 引入头文件
```cpp
#include "smart_ptr_fixed.h"
```

### 2. 使用智能指针

#### shared_ptr（共享所有权）
```cpp
smart_ptr::shared_ptr<MyClass> sp1(new MyClass(42));
smart_ptr::shared_ptr<MyClass> sp2 = sp1;  // 引用计数 +1
printf("use_count: %d\n", sp1.use_count());  // 输出: 2
```

#### weak_ptr（弱引用，不增加引用计数）
```cpp
smart_ptr::weak_ptr<MyClass> wp(sp1);
if (!wp.expired()) {
    smart_ptr::shared_ptr<MyClass> sp3 = wp.lock();
    // 使用 sp3...
}
```

#### unique_ptr（独占所有权）
```cpp
smart_ptr::unique_ptr<MyClass> up(new MyClass(42));
up->DoSomething();
up.reset();  // 手动释放
```

#### shared_array（共享数组）
```cpp
smart_ptr::shared_array<int> arr(new int[10]);
arr[0] = 100;
smart_ptr::shared_array<int> arr2 = arr;  // 共享数组所有权
```

#### make_shared（工厂函数）- STL 风格
```cpp
// 无参数
auto sp1 = smart_ptr::make_shared<MyClass>();

// 带参数
auto sp2 = smart_ptr::make_shared<MyClass>(42, "hello");
```

#### make_unique（工厂函数）- STL 风格
```cpp
// 无参数
auto up1 = smart_ptr::make_unique<MyClass>();

// 带参数
auto up2 = smart_ptr::make_unique<MyClass>(42);
```

## 构建和测试

### 环境要求
- Windows 10
- Visual Studio 2019 (MSVC v142)
- Python 3（可选，用于编码转换）

### 快速开始

#### 一键构建
```batch
build.bat
```

#### 运行演示和测试
```batch
test_run.bat
```

#### 手动构建
```batch
:: 设置 VS2019 环境
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"

:: 编译演示程序（C++98）
cl -nologo -W4 -EHsc -O2 demo.cpp

:: 编译测试程序（C++11，catch2 需要）
cl -nologo -W4 -EHsc -O2 test_smart_ptr.cpp
```

## 兼容性

- **生产代码**：C++98/C++03（VS2005 - VS2022 兼容）
- **测试代码**：C++11（catch2 框架需要）

## 设计特点

1. **非侵入式**：不需要修改被管理的类
2. **引用计数**：使用独立的 `ref_count` 类管理引用计数
3. **强/弱引用分离**：支持 `shared_ptr`（强引用）和 `weak_ptr`（弱引用）
4. **类型安全**：支持多态和类型转换
5. **COM 支持**：提供 `com_mem_mgr` 用于 COM 对象管理

## 新增 STL 兼容接口

### 比较操作符
```cpp
smart_ptr::shared_ptr<T> sp1, sp2;
if (sp1 == sp2) { }  // 比较指针值
if (sp1 != sp2) { }
if (sp1 < sp2)  { }  // 用于关联容器
```

### 指针转换（类似 std::static_pointer_cast 等）
```cpp
// 静态转换
auto sp_derived = smart_ptr::static_pointer_cast<Derived>(sp_base);

// 动态转换
auto sp_derived = smart_ptr::dynamic_pointer_cast<Derived>(sp_base);

// const 转换
auto sp_nonconst = smart_ptr::const_pointer_cast<T>(sp_const);
```

### swap 函数
```cpp
smart_ptr::swap(sp1, sp2);  // 自由函数，类似 std::swap
```

### unique_ptr::release()
```cpp
T* raw = up.release();  // 释放所有权并返回原始指针，不删除对象
```

## 移动语义（自动检测）

根据编译器版本自动启用 C++11 移动语义：

- **C++11 及以上**（`__cplusplus >= 201103L` 或 MSVC >= 1900）：自动启用 move 构造函数和 move 赋值运算符
- **C++98/03**：禁用 move 语义，保持拷贝语义兼容

```cpp
#include "smart_ptr_fixed.h"

// C++11 环境下可以正常使用 move
smart_ptr::unique_ptr<T> up1(new T);
smart_ptr::unique_ptr<T> up2 = std::move(up1);  // 自动启用
```

**注意**：`unique_ptr` 在 C++98 中通过私有拷贝构造函数禁止拷贝，C++11 中则启用 move 语义实现所有权转移。

## 注意事项

1. **不支持循环引用**：如果两个对象互相持有 `shared_ptr`，会导致内存泄漏。应使用 `weak_ptr` 打破循环
2. **线程安全**：当前实现非线程安全，如需多线程使用需要额外同步
3. **数组支持**：使用 `shared_array` 管理数组，`shared_ptr` 不支持数组（不会调用 `delete[]`）
