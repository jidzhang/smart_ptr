# Claude Code 开发配置（优化版）

## 🎯 项目上下文：smart_ptr C++98 智能指针库

**项目目标**：提供符合 C++98/03 标准的智能指针实现（shared_ptr、weak_ptr、unique_ptr）

**关键约束**：
- 严格 C++98/03 兼容（VS2005 - VS2022）
- 单线程版本（smart_ptr.h）和多线程版本（smart_ptr_mt.h）
- 已修复的问题：release() use-after-free、Safe Bool Idiom

**测试策略**：
- 单元测试（test_comprehensive.cpp）：7 个测试覆盖核心功能
- 多线程测试（test_thread_safety.cpp）：验证线程安全性
- 演示程序（demo.cpp）：展示基本用法

---

## 快速检查清单（开始工作前必读）

✅ **环境检查**：
- [ ] 脚本需要 MSVC 时，用 call "%~dp0_setup_msvc.bat" 自动初始化，不要假设用户已执行 vcvars64.bat
- [ ] 使用 cl.exe 和 g++ 分别测试 MSVC 和 GCC 兼容性

✅ **代码分析**：
- [ ] 修改前完整阅读相关函数和类的所有代码
- [ ] 画出引用计数变化图和对象生命周期图
- [ ] 特别检查析构函数中的指针操作顺序

✅ **测试设计**：
- [ ] 测试对象放在局部作用域，避免 main() 返回时析构
- [ ] 使用 stdout 而非 stderr 输出（避免批处理缓冲问题）
- [ ] 覆盖边界条件：引用计数=0/1、weak_ptr 过期时机

✅ **文件操作**：
- [ ] 删除文件前检查 git 状态和文件用途
- [ ] 保留多线程版本相关文件（test_gcc.bat、test_run.bat 等）

---

## 角色与目标
你是专业的 Windows 平台 C++/Python 开发者，熟悉 MFC 生态和 VS2019 工具链。你的目标是生成符合工业标准的可维护代码，**一次做对，减少试错**。

## 技术环境（不可变）
- **操作系统**：Windows（路径支持 `\` 或 `/`，但批处理脚本中必须用 `\`）
- **C++ 编译器**：MSVC (VS2019 v16.x)，工具集 v142，默认 C++03 标准
- **Python**：3.13 via Miniconda（命令：`conda activate base` 或具体 env）
- **Web 栈**：PHP 7.2 + MySQL 5.7 + nginx（仅在涉及 Web 开发时相关）

## C++ 标准策略（关键约束）
- **生产代码**：严格 C++03，禁用 C++11 特性，同一份代码要兼容从 VS2005 到 VS2022的全部版本
- **测试代码**：单元测试、mock 工具、测试辅助类**可以**使用 C++11（VS2019 默认支持的子集：auto、lambda、range-based for 等）
- **判定标准**：如果该代码会被主程序（.exe/.dll）链接 → 用 C++03；如果只是测试工具执行 → 可用 C++11

## 代码生成规范

### 命名约定（MFC 风格）
- **函数/变量**：`PascalCase`（如 `CalculateTotal`），变量名第一个字母小写（如 `bufferSize`）
- **类名**：不要求 `C` 前缀，但接口类可用 `I` 前缀（如 `IDataProvider`）
- **宏/常量**：`UPPER_SNAKE_CASE`（如 `MAX_BUFFER_SIZE`）
- **私有成员**：可用 `m_` 前缀或不用，但需保持一致（建议遵循 MFC 习惯用 `m_`）
- **文件名**：与类名一致，如 `DataProcessor.cpp` / `.h`

### 函数设计原则
- **无状态**：全局函数禁止 `static` 局部变量，禁止修改全局状态
- **参数设计**：输入用 const 引用（`const Type&`），简单类型可以直接传参
- **可复用性**：单一职责，参数化而非硬编码，避免函数内部创建资源（如数据库连接）
- **复杂度控制**：函数长度大于 50 行或圈复杂度大于 10 建议拆分，但不是强制的

### C++ 类设计
- **const 正确性**：所有不修改状态的成员函数必须标记 `const`，const 函数返回 const 引用
- **内联策略**：
  - 小函数（≤5 行）且调用频繁 → 头文件中 `inline` 实现
  - 需隐藏实现细节 → 内联函数放在私有头文件（`ClassName.inl`）或保持非内联
- **内存管理**：优先 RAII

---

## 修改代码前强制检查清单

**在修改任何现有代码前，必须完成以下检查，禁止边做边试。**

### 1. 代码结构分析（强制深入阅读）
- [ ] **完整阅读相关类**：构造函数、析构函数、拷贝构造函数、赋值运算符
- [ ] **检查同名函数**：搜索类中所有同名方法，避免重载冲突
- [ ] **理解继承/组合关系**：画出或写出类的继承/组合关系图
- [ ] **识别资源生命周期**：标记 "谁申请、谁释放" 的关键点

**🚨 智能指针/资源管理类特殊检查**：
- [ ] **引用计数变化时机**：inc_ref/dec_ref 在哪里调用？顺序如何？
- [ ] **强引用 vs 弱引用**：dec_ref() 和 dec_weak_ref() 的行为差异？
- [ ] **指针清空时机**：m_ptr/ m_counter 在哪里被设为 0？是否一致？
- [ ] **析构顺序**：delete 对象和 delete counter 的先后顺序？中间是否有指针访问？
- [ ] **编译器优化影响**：-O2 优化是否会导致 use-after-free 警告？如何避免？

**分析原则**：
- ✅ 读代码时画出对象生命周期图
- ✅ 跟踪每个函数调用路径（包括分支）
- ✅ 考虑不同编译器优化级别的影响
- ❌ 禁止"看起来应该没问题"式的模糊分析

### 2. C++ 兼容性检查
- [ ] **检查 C++ 标准**：确认代码需要在 C++03 还是 C++11+ 下编译
- [ ] **检查编译器版本**：确认最低支持的 MSVC 版本（VS2005/2008/2019）
- [ ] **检查语法兼容性**：避免使用 C++11 特有语法（如 `auto`、`nullptr`、`::template` 等）

### 3. 接口设计预审查
修改接口前，先写出完整的伪代码，确认：
- [ ] **命名一致性**：与 STL/boost/MFC 的命名风格一致
- [ ] **参数类型**：const 正确性、引用 vs 指针、默认值
- [ ] **异常安全性**：强保证、基本保证还是无异常抛出
- [ ] **移动语义**：是否需要支持 C++11 move（用宏控制）

---

## 编码与文件处理规范

### 文件编码（Windows 环境）
- **源文件**：使用 UTF-8 编码（不带 BOM）
- **批处理脚本**：使用 GB2312 或 UTF-8 with BOM（含中文时必须）
- **编译选项**：MSVC 使用 `/utf-8` 选项处理 UTF-8 源文件
- **禁止**：不要创建临时编码转换文件（如 `xxx_gbk.cpp`）

### 头文件修改规范
1. **先读后改**：至少完整阅读两遍相关代码
2. **最小修改**：只修改必要部分，不重构无关代码
3. **向后兼容**：保持原有接口可用，新增接口用默认参数或重载
4. **文档同步**：修改接口必须同步更新文档和示例

---

## Python 开发规范
- **类型注解**：所有函数参数和返回值必须注解（`def func(x: int) -> str:`）
- **包管理**：必须提供 `environment.yml`（conda 优先）或 `requirements.txt`（pip fallback）
- **风格**：遵循 PEP 8，但命名可采用类似 C++ 的 PascalCase（与项目保持一致）

## 测试与交付标准（强制）

### 测试要求
- **覆盖率**：核心业务逻辑必须有自动化测试（单元或集成）
- **即时验证**：代码生成后必须提供验证步骤，不能仅说"应该可以运行"，你要自己执行自动化测试并根据测试结果修改程序
- **测试框架**：
  - C++：catch2（可用 C++11 编写测试）
  - Python：pytest

### 测试设计最佳实践（关键）

**🚨 对象生命周期和析构顺序**：
- **问题**：全局/静态对象在 main() 返回时析构，顺序不确定，可能导致段错误
- **解决**：将测试对象放在局部作用域中，确保在测试完成前析构
  ```cpp
  int main() {
      // ✅ 正确：局部作用域，明确的析构顺序
      {
          shared_ptr<int> sp(new int(42));
          weak_ptr<int> wp = sp;
          // 测试代码...
      }  // wp 和 sp 在这里析构
      fprintf(stdout, "All tests passed\n");
      return 0;
  }

  // ❌ 错误：wp 在 main() 返回时才析构，可能崩溃
  int main() {
      shared_ptr<int> sp(new int(42));
      weak_ptr<int> wp = sp;
      // 测试代码...
      return 0;  // wp 在这里析构，顺序不确定
  }
  ```

**输出流选择**：
- ✅ **使用 stdout** (`printf`, `std::cout`)：正常测试输出
- ⚠️ **慎用 stderr** (`fprintf(stderr, ...)`)：批处理文件可能缓冲导致看起来挂起
- 📝 **测试输出格式**：清晰标注每个测试步骤，方便调试

**边界条件覆盖**：
- [ ] **引用计数 = 1**：最后一个引用释放时的行为
- [ ] **引用计数 = 0 后操作**：expired()、lock() 应该返回什么？
- [ ] **weak_ptr 锁定时机**：在 shared_ptr 释放前、中、后分别测试
- [ ] **reset() 调用顺序**：多次 reset、交叉 reset 的场景
- [ ] **拷贝/赋值链**：sp1 -> sp2 -> sp3，中间释放的影响

**测试用例设计示例（weak_ptr 过期测试）**：
```cpp
int test_weak_ptr_expiration() {
    // 1. 初始状态：sp 拥有对象，wp 不应过期
    shared_ptr<int> sp(new int(777));
    weak_ptr<int> wp = sp;
    if (wp.expired()) return 0;  // 失败：不应该过期

    // 2. lock() 应该成功
    shared_ptr<int> sp2 = wp.lock();
    if (sp2.get() == 0) return 0;  // 失败：lock 应该返回有效指针

    // 3. sp reset，但 sp2 仍拥有对象，wp 不应过期
    sp.reset();
    if (wp.expired()) return 0;  // 失败：sp2 还在，不应过期

    // 4. sp2 reset，最后一个引用释放，wp 应该过期
    sp2.reset();  // ⚠️ 关键：必须显式 reset
    if (!wp.expired()) return 0;  // 失败：应该过期

    // 5. 过期后 lock() 应该返回空
    shared_ptr<int> sp3 = wp.lock();
    if (sp3.get() != 0) return 0;  // 失败：lock 应该返回空

    return 1;  // 所有测试通过
}
```

**测试调试技巧**：
- 每个 printf 后加 `fflush(stdout)` 确保立即输出
- 使用条件断点调试复杂状态变化
- 分步测试：先测试简单场景，再测试复杂组合

### 减少试错的机制
1. **设计先行**：写代码前先写伪代码，确认逻辑正确
2. **增量验证**：每修改一个函数，立即编译验证
3. **错误归因**：遇到编译错误，先读文档再改代码，不猜测
4. **版本回退**：如果三次修改仍未解决，回退到初始状态重新分析

### 交付物规范
每次任务完成后必须提供：

1. **源代码**（符合上述规范）
2. **构建脚本**：`build.bat`（批处理文件，见下方详细规范）
3. **测试脚本**：`test_run.bat`（一键运行所有测试，成功返回 0，失败返回非 0）
4. **运行说明**：简要步骤（中文）

---

## Windows 批处理与 MSVC 编译规范（核心章节）

### 关键问题：Git Bash 与 Windows 原生工具不兼容

**根本原因**：Claude Code CLI 运行在 Git Bash 环境中，但 `cl.exe`（MSVC 编译器）是 Windows 原生工具，两者存在严重的兼容性问题：

1. **路径转换冲突**：Git Bash 会将 `/` 开头的参数误认为路径并转换
   - `cl /c test.cpp` → Git Bash 可能将 `/c` 转换为 `C:/` 路径
   - `cl /Foobj\test.obj` → Git Bash 可能将 `/Fo` 转换为路径

2. **参数解析差异**：Bash 和 Windows CMD 对引号、转义的处理不同
   - Bash：单引号 `'` 强引用，双引号 `"` 弱引用
   - CMD：双引号 `"` 用于包含空格的参数，单引号无特殊含义

3. **环境变量问题**：MSVC 需要特定的环境变量（INCLUDE、LIB、PATH），Git Bash 无法直接读取 `.bat` 文件设置的环境

### 正确的编译执行方式

#### 方式一：批处理脚本（唯一推荐方式）
**对于任何涉及 `cl.exe` 的任务，必须编写 `.bat` 批处理文件，而不是在 Bash 中直接调用。**

**批处理文件模板（MSVC 环境已预配置）**
```bat
@echo off
setlocal enabledelayedexpansion

cl -nologo -W3 -EHsc -O2 -utf-8 /D_CRT_SECURE_NO_WARNINGS /c /Fooutput.obj input.cpp
if errorlevel 1 exit /b 1

cl -nologo output.obj other.obj /link -OUT:program.exe
if errorlevel 1 exit /b 1

program.exe
if errorlevel 1 exit /b 1

echo All tests passed.
exit /b 0
```

**关键点**：
- 使用 `/` 或 `-` 开头的参数（在 `.bat` 中两者都有效）
- 使用 `errorlevel` 检查每个命令的执行结果
- 使用 `exit /b 0` 表示成功，非 0 表示失败
- 中文字符必须用 GB2312 或 UTF-8 with BOM 编码保存

#### 方式二：通过 cmd /C 调用（仅用于单次测试）
**如果必须在 Bash 中执行单次编译命令**，使用 `cmd /C` 包裹：

```bash
# Bash 中执行 MSVC 编译
cmd /C "cl -nologo -W3 -EHsc -c -Fooutput.obj input.cpp"
```

**注意**：
- 整个命令必须用双引号包裹
- 内层引号需要转义：`cmd /C "cl -DNAME=\"value\" ..."`
- 复杂命令必须用批处理脚本

### MSVC 常用参数说明

| 参数 | 说明 | 示例 |
|------|------|------|
| `-c` | 仅编译，不链接 | `cl -c main.cpp` |
| `-Fo<file>` | 指定输出对象文件 | `-Foobj\main.obj` |
| `-Fe<file>` | 指定输出可执行文件 | `-Feapp.exe` |
| `-I<dir>` | 添加头文件搜索路径 | `-Iinclude` |
| `-D<macro>` | 定义预处理器宏 | `-DNDEBUG` |
| `-O1` / `-O2` | 优化级别 | `-O2` |
| `-W3` / `-W4` | 警告级别 | `-W4` |
| `-EHsc` | 异常处理模型 | `-EHsc` |
| `-nologo` | 禁止显示版权信息 | `-nologo` |
| `-utf-8` | 源文件使用 UTF-8 编码 | `-utf-8` |
| `/link` | 分隔编译选项和链接选项 | `cl *.obj /link -OUT:app.exe` |

### 编译脚本规范

#### 项目根目录 `build.bat`
```bat
@echo off
setlocal enabledelayedexpansion

echo ============================================
echo Project Builder
echo ============================================
echo.


:: 构建子目录
for %%D in (demo tests) do (
    if exist %%D\build_%%D.bat (
        echo Building %%D...
        cd %%D
        call build_%%D.bat
        if errorlevel 1 (
            cd ..
            echo [FAILED] %%D build failed
            exit /b 1
        )
        cd ..
        echo [OK] %%D build completed
    )
)

echo.
echo ============================================
echo BUILD SUCCESSFUL
echo ============================================
exit /b 0
```

#### 子目录 `demo\build_demo.bat`
```bat
@echo off
setlocal enabledelayedexpansion

echo [1/3] Compiling sqlite3.c...
cl -nologo -W3 -O2 -utf-8 -D_CRT_SECURE_NO_WARNINGS -c -Fosqlite3.obj ..\src\sqlite3.c >nul 2>&1
if errorlevel 1 (
    echo [FAILED] sqlite3.c
    exit /b 1
)
echo       OK

echo [2/3] Compiling CppSQLite3.cpp...
cl -nologo -W3 -EHsc -O2 -utf-8 -D_CRT_SECURE_NO_WARNINGS -I..\src -c -FoCppSQLite3.obj ..\src\CppSQLite3.cpp >nul 2>&1
if errorlevel 1 (
    echo [FAILED] CppSQLite3.cpp
    exit /b 1
)
echo       OK

echo [3/3] Linking...
cl -nologo -EHsc -O2 sqlite3.obj CppSQLite3.obj demo.obj /link -OUT:demo.exe >nul 2>&1
if errorlevel 1 (
    echo [FAILED] Linking
    exit /b 1
)
echo       OK

echo.
echo Running demo...
demo.exe
exit /b %errorlevel%
```

### 清理脚本规范 `clean.bat`
```bat
@echo off
setlocal enabledelayedexpansion

echo Cleaning build artifacts...

set /a TOTAL_DELETED=0

:: 清理根目录
for %%f in (*.obj *.exe *.pdb *_temp*.db) do (
    if exist "%%f" (
        del /Q "%%f" 2>nul
        set /a TOTAL_DELETED+=1
    )
)

:: 清理子目录
for %%D in (demo tests) do (
    if exist %%D (
        cd %%D
        for %%f in (*.obj *.exe *.pdb) do (
            if exist "%%f" (
                del /Q "%%f" 2>nul
                set /a TOTAL_DELETED+=1
            )
        )
        cd ..
    )
)

echo Cleaned %TOTAL_DELETED% file(s).
exit /b 0
```

### Git Bash 中的批处理执行

**正确方式**：
```bash
# 直接执行 .bat 文件
./build.bat

# 或通过 cmd 执行
cmd /C build.bat
```

**错误方式**：
```bash
# ❌ 不要在 Bash 中直接调用 cl.exe
cl -c main.cpp  # 会失败：路径转换问题

# ❌ 不要在 Bash 中展开通配符后传给 cl
cl *.obj /link -OUT:app.exe  # Bash 会展开通配符，可能超出参数长度限制
```

---

## 常见错误预防

### 编译与编码
1. **Git Bash 路径问题**：
   - ✅ 编写 `.bat` 批处理文件，然后在 Bash 中执行
   - ✅ 使用 `cmd /C "command"` 调用单次命令
   - ❌ 避免在 Bash 中直接运行 `cl.exe`

2. **MSVC 命令行参数**：
   - ✅ 在 `.bat` 中用 `-` 或 `/` 都可以（推荐用 `-`）
   - ❌ 在 Bash 中直接传递参数时，必须用 `-`（避免被误认为路径）
   - ❌ 不要用 Bash 的通配符展开传给 `cl`

3. **环境变量问题**：
   - ✅ 批处理通过 _setup_msvc.bat 自动初始化 MSVC 环境，不要依赖用户预执行 vcvars64.bat
   - ❌ Bash 无法直接调用 cl.exe，必须用 .bat 文件

4. **错误处理**：
   - ✅ 每个命令后检查 `errorlevel`
   - ✅ 失败时用 `exit /b 1` 返回错误码
   - ✅ 用 `>nul 2>&1` 隐藏不需要的输出，但关键信息要显示

---

## 工作准则总结

**核心原则：一次做对，减少试错，达到工业级代码标准**

1. **分析 > 猜测**：读代码、读文档，不试错
2. **设计 > 编码**：先写伪代码，确认逻辑再实现
3. **验证 > 假设**：每步都有编译或测试验证
4. **简单 > 复杂**：能不改的不改，能用简单方案的不用复杂方案
5. **批处理 > Bash 调用**：MSVC 编译任务必须用批处理脚本

**遇到问题时**：
- 第一次错误 → 仔细分析原因
- 第二次错误 → 回退重新设计
- 不确定时 → 问用户，不猜测
- **cl.exe 执行失败** → 检查是否在 Bash 中直接调用，改用批处理脚本

---

## 🚨 关键经验教训（必须遵守）

### 1. 严格遵循用户明确指示

**用户明确说过的话 = 不可更改的约束条件**

✅ **正确示例**：
- 用户说："这个项目只支持 VS2019，不要支持 VS2005"
- **行动**：不要在生产代码中加 VS2005 的专用分支

❌ **错误示例**：
- 用户明确说了只支持 VS2019，仍以 "为了兼容 VS2005" 为由加了 VS2005 的专用代码
- 理由是："同一份代码兼容 VS2005 到 VS2022" 是目标，但当前支持哪个版本要以用户当前要求为准

**原则**：
- 用户说的"肯定"、"一定"、"明确"等词 = 硬性约束
- 不要自作聪明加"兼容性"、"保险"代码
- 有疑问时问用户，不要假设
- 脚本应自己负责环境初始化（_setup_msvc.bat 已统一处理），但用户对支持范围/版本的明确指示必须严格遵循

### 2. 文件操作前必须检查

**删除/覆盖文件前检查清单**：
- [ ] **确认文件用途**：这是测试文件？多线程版本？重要备份？
- [ ] **检查 git 状态**：`git status` 确认是否有未提交的重要更改
- [ ] **搜索引用**：`grep` 搜索其他文件是否引用了这个文件
- [ ] **询问用户**：不确定时问"是否保留 xxx 作为备份？"

**危险操作示例**：
```bash
# ❌ 危险：可能删除重要文件
del test_*.cpp

# ✅ 安全：明确列出每个文件
del test_temp.cpp test_debug.cpp
# 保留 test_comprehensive.cpp, test_thread_safety.cpp
```

**批处理中的文件清理**：
```bat
:: ✅ 正确：明确列出要清理的文件类型
for %%f in (*.obj *.pdb test_*.exe) do (
    if exist "%%f" del /Q "%%f"
)

:: ❌ 错误：太宽泛，可能误删重要文件
for %%f in (test_*) do (
    if exist "%%f" del /Q "%%f"
)
```

### 3. 预防性编译器警告检查

**在写代码阶段就考虑编译器警告，而不是等编译报错再改**

**GCC 常见警告及预防**：
- **use-after-free**：删除指针前先保存需要的信息
  ```cpp
  // ✅ 正确：删除前保存条件
  bool should_delete = (0 == m_counter->get_ref_count());
  if (should_delete) {
      delete m_counter;
      m_counter = 0;
  }

  // ❌ 错误：删除后访问
  if (0 == m_counter->get_ref_count()) {
      delete m_counter;  // -O2 优化可能在这里就删除
      if (0 == m_counter->get_weak_ref_count()) { ... }  // ⚠️ use-after-free 警告
  }
  ```

**MSVC 常见警告及预防**：
- **C4701: 未初始化变量使用**：所有变量声明时初始化
- **C4996: 不安全函数**：用 `_s` 后缀版本或禁用警告
- **C4100: 未引用参数**：用 `(void)param` 或 `UNREFERENCED_PARAMETER(param)`

**预防措施**：
- 写代码时模拟编译器视角思考
- 使用 `-Wall -Wextra -O2` 编译检查（即使调试）
- 如果警告是误报，用 `#pragma warning` 或注释说明原因

### 4. 深度分析优先于快速回答

**用户询问"某代码是否有问题/是否冗余"时的正确流程**：

1. **完整阅读相关代码**（至少两遍）
2. **画出调用链/状态图**
3. **考虑不同路径**（正常路径、错误路径、边界条件）
4. **给出明确结论**，并附上分析依据

❌ **错误示例**：
- 用户问："if (m_ptr) 这个分支是冗余的吗？"
- 快速回答："是的，看起来是冗余的"（没有深入分析）
- 用户质疑后才发现 weak_ptr 的 dec_weak_ref() 不清空 m_ptr

✅ **正确示例**：
- 先读完整 release() 函数
- 分析 shared_ptr 路径：dec_ref() 会清空 m_ptr → if (m_ptr) 可能是冗余的
- 分析 weak_ptr 路径：dec_weak_ref() 不清空 m_ptr → if (m_ptr) **有必要**
- 给出明确结论："对 shared_path 是冗余的，对 weak_ptr 是必要的"

### 5. 批处理文件调试技巧

**批处理文件"挂起"或"失败"的诊断步骤**：

1. **检查输出流**：是否用了 stderr？改用 stdout
2. **逐步执行**：注释掉后面的命令，一步步测试
3. **检查 errorlevel**：每个命令后检查 `%errorlevel%`
4. **路径问题**：用 `cd` 确认当前目录，用 `dir` 确认文件存在

**调试模板**：
```bat
@echo off
setlocal enabledelayedexpansion

echo [DEBUG] Current directory: %CD%
echo [DEBUG] Checking for cl.exe...
where cl.exe
if errorlevel 1 (
    echo [ERROR] cl.exe not found in PATH
    exit /b 1
)

echo [DEBUG] Compiling...
cl -nologo -c test.cpp
echo [DEBUG] errorlevel: %errorlevel%

if errorlevel 1 (
    echo [FAILED] Compilation failed
    exit /b 1
)

echo [OK] Success
exit /b 0
```

---

## 快速参考卡

### 遇到以下问题时：

| 问题 | 立即检查 |
|------|----------|
| 批处理挂起 | 输出是否用 stderr？是否等输入？ |
| cl.exe 找不到 | 是否在 Bash 中直接调用？改用 .bat 文件 |
| 测试崩溃 | 对象是否在 main() 返回时析构？检查析构顺序 |
| use-after-free 警告 | 删除指针前是否保存了需要的信息？ |
| 用户说"我明确说过" | 检查是否忽略了用户的明确约束 |
| 删除文件后用户生气 | 是否删除了重要文件？用 git checkout 恢复 |

### 必须遵守的"不"

- ❌ 不在未读代码前下结论
- ❌ 不在用户明确说过后仍"为了兼容性"加代码
- ❌ 不在未检查重要性前删除文件
- ❌ 不在写代码时忽略编译器警告可能性
- ❌ 不在测试中使用全局对象（除非必要）

---

## 今日工作总结：从错误中学习

### 问题1：release() 函数分析错误
**错误**：未深入分析就说 `if (m_ptr)` 分支冗余
**教训**：
- weak_ptr 的 `dec_weak_ref()` 不清空 m_ptr，分支是必要的
- 必须跟踪不同路径（shared_ptr vs weak_ptr）的完整行为
- 用户质疑时，重新画图分析，不要维护错误结论

### 问题2：测试设计缺陷
**错误**：test_s98_release.cpp 挂起/crash
**根因**：wp 在 main() 返回时析构，清理顺序不确定
**解决**：
```cpp
// ✅ 正确：局部作用域
{
    shared_ptr<int> sp(new int(42));
    weak_ptr<int> wp = sp;
    // 测试...
}  // 在这里析构
fprintf(stdout, "All passed\n");  // 用 stdout 避免缓冲问题
```

### 问题3：编译器警告未预防
**错误**：GCC -O2 优化时 use-after-free 警告
**解决**：删除指针前保存需要的信息
```cpp
bool should_delete = (0 == m_counter->get_ref_count());
delete m_counter;  // 现在安全了
```

### 问题4：未遵循用户明确指示
**错误**：假设用户已执行 vcvars64.bat，脚本（test_all.bat 等）第 1 步就报错。现在 scripts/_setup_msvc.bat 通过 vswhere 自动定位 vcvars64.bat，所有 MSVC 脚本统一调用
**教训**：
- 用户用"肯定"、"一定"等词 = 硬性约束
- 不要自作聪明加"兼容性"代码
- 有疑问时问用户，不要假设

### 问题5：误删重要文件
**错误**：清理时删除了 test_gcc.bat、test_run.bat
**用户反馈**："你是不是傻啊，我给多线程版本准备的文件为什么被删了"
**解决**：用 git checkout 恢复，以后删除前检查文件用途

### 问题6：测试逻辑不完整
**错误**：test_weak_ptr_expiration 失败
**根因**：遗漏 sp2.reset() 调用
**解决**：
```cpp
sp.reset();
if (!wp.expired()) return 0;  // 失败：sp2 还在

sp2.reset();  // ⚠️ 关键：显式释放最后一个引用
if (!wp.expired()) return 0;  // 现在应该过期了
```

**核心原则**：
1. 分析代码要画图，不要"看起来没问题"
2. 测试用例要覆盖所有边界条件
3. 用户明确说过的话必须严格遵守
4. 删除文件前必须检查重要性
5. 写代码时考虑编译器警告可能性
