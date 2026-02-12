# Claude Code 开发配置（优化版）

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

### 1. 代码结构分析
- [ ] **阅读构造函数**：完整阅读目标类的构造函数、析构函数、拷贝构造函数
- [ ] **检查同名函数**：搜索类中所有同名方法，避免重载冲突
- [ ] **理解继承关系**：画出或写出类的继承/组合关系图
- [ ] **识别资源生命周期**：标记 "谁申请、谁释放" 的关键点

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

#### 方式一：批处理脚本（推荐）
**对于任何涉及 `cl.exe` 的任务，必须编写 `.bat` 批处理文件，而不是在 Bash 中直接调用。**

```bat
@echo off
setlocal enabledelayedexpansion

:: 初始化 MSVC 环境（VS2019 示例）
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1

:: 编译命令（使用 MSVC 原生参数风格）
cl /nologo /W3 /EHsc /O2 /utf-8 /D_CRT_SECURE_NO_WARNINGS /c /Fooutput.obj input.cpp
if errorlevel 1 exit /b 1

:: 链接
cl /nologo output.obj other.obj /link -OUT:program.exe
if errorlevel 1 exit /b 1

:: 运行测试
program.exe
if errorlevel 1 exit /b 1

echo All tests passed.
exit /b 0
```

**关键点**：
- 使用 `/` 或 `-` 开头的参数（在 `.bat` 中两者都有效，推荐用 `-` 避免 Git Bash 误解析）
- 使用 `errorlevel` 检查每个命令的执行结果
- 使用 `exit /b 0` 表示成功，非 0 表示失败
- 中文字符必须用 GB2312 或 UTF-8 with BOM 编码保存

#### 方式二：通过 cmd /C 调用
**如果必须在 Bash 中执行单次编译命令**，使用 `cmd /C` 包裹：

```bash
# Bash 中执行 MSVC 编译
cmd /C "cl -nologo -W3 -EHsc -c -Fooutput.obj input.cpp"
```

**注意**：
- 整个命令必须用双引号包裹
- 内层引号需要转义：`cmd /C "cl -DNAME=\"value\" ..."`
- 复杂命令建议用批处理脚本，而不是 cmd /C

#### 方式三：通过 PowerShell 调用
```bash
powershell -Command "& {cl -nologo -W3 -EHsc -c -Fooutput.obj input.cpp}"
```

**缺点**：PowerShell 对某些 MSVC 参数（如 `;` 分隔的列表）有特殊处理，不如批处理稳定。

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

:: 初始化 MSVC 环境
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Failed to initialize MSVC environment
    exit /b 1
)

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
   - ✅ 在 `.bat` 中用 `call vcvars64.bat` 初始化环境
   - ❌ Bash 无法继承 `.bat` 中设置的环境变量

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
