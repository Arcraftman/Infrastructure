# Infra

一个用 C（及可选 C++）编写的模块化基础设施工具库，包含 `stk`（数据结构/容器）、
`lnx`（Linux 系统接口封装）和 `web`（HTTP/WebSocket 服务端与客户端）三个模块。

本项目通过 CMake 构建，支持静态库与共享库，并提供了跨平台的导出宏
（不依赖 `GenerateExportHeader`，不生成 `infra_xxx_export.h`）。

---

## 1. 前置要求

| 工具 | 版本 |
|---|---|
| CMake | >= 3.19 |
| C 编译器 | GCC / Clang（Linux）；MSVC / MinGW-w64（Windows，导出宏已支持） |
| 构建工具 | Ninja 或 Unix Makefiles（任选） |
| 可选 | Doxygen（生成 API 文档）、lcov/gcovr（覆盖率） |

> 当前 `lnx` 模块为 Linux-only（其 CMake 在非 Linux 平台会被跳过）。
> `web` 在 Linux 下使用 epoll，并保留 select 回退路径。

---

## 2. 快速开始

最简方式（使用 CMake Presets）：

```bash
# 配置 + 构建 Debug（Ninja）
cmake --preset linux-debug-ninja
cmake --build --preset linux-debug-ninja -j$(nproc)

# 运行测试
ctest --test-dir build/linux-debug-ninja --output-on-failure
```

> 注意：构建**必须显式指定要编译的模块**（见第 3 节）。Preset 默认不携带模块列表，
> 直接 `--preset` 会触发「未指定模块」提示并空构建。请在 preset 后追加
> `-DINFRA_MODULES='stk;lnx;web'`。

---

## 3. 配置（CMake 选项）

### 3.1 模块选择（必填）

必须通过 `-DINFRA_MODULES` 指定要构建的模块，未指定时仅打印警告并构建空工程：

```bash
-DINFRA_MODULES='stk;lnx;web'
```

**模块名出现在 `INFRA_MODULES` 中即自动启用**，无需再单独传 `-DINFRA_ENABLE_*=ON`。
要排除某模块，直接不写进列表即可，例如只构建 `stk;web`。

可用模块：

| 模块 | 说明 | 平台 |
|---|---|---|
| `stk` | 数据结构/容器（vector/list/rbtree/hashmap/...） | 跨平台 |
| `lnx` | Linux 系统接口（文件/进程/信号/监控） | Linux only |
| `web` | HTTP/WebSocket 服务端 + 客户端 | Linux（epoll） |

### 3.2 库类型

```bash
-DINFRA_LIBRARY_TYPE=SHARED   # 默认，共享库
-DINFRA_LIBRARY_TYPE=STATIC   # 静态库
```

### 3.3 测试 / 示例 / 文档

```bash
-DINFRA_BUILD_TESTS=ON    # 默认 ON，构建单元测试
-DINFRA_BUILD_EXAMPLES=ON # 默认 OFF，构建示例（examples 目录存在 CMakeLists 才添加）
-DINFRA_BUILD_DOCS=ON     # 默认 OFF，需 Doxygen；未安装则安全跳过
```

### 3.4 质量与调试

```bash
-DINFRA_ENABLE_ASAN=ON          # AddressSanitizer（默认 OFF）
-DINFRA_ENABLE_COVERAGE=ON      # 覆盖率插桩（默认 OFF）
-DINFRA_ENABLE_STRICT_WARNINGS=ON # -Werror（默认 OFF，勿与 ASan 同开）
-DINFRA_ENABLE_WARNINGS=ON      # -Wall -Wextra（默认 ON）
```

### 3.5 安装

```bash
-DINFRA_INSTALL=ON              # 默认 ON
-DINFRA_INSTALL_CMAKE_CONFIG=ON # 生成 find_package(Infra) 配置（默认 ON）
-DINFRA_INSTALL_PKGCONFIG=ON    # 生成 .pc 文件（默认 ON）
```

---

## 4. 构建方式

### 方式 A：CMake Presets（推荐）

可用预设：

| Preset | 说明 |
|---|---|
| `linux-debug-ninja` | Debug + Ninja |
| `linux-release-ninja` | Release + Ninja |
| `linux-debug-make` | Debug + Unix Makefiles |
| `linux-release-make` | Release + Unix Makefiles |
| `linux-debug-static-ninja` | Debug 静态库 |
| `linux-asan-ninja` | Debug + AddressSanitizer |

示例（带模块）：

```bash
cmake --preset linux-debug-ninja -DINFRA_MODULES='stk;lnx;web'
cmake --build --preset linux-debug-ninja -j$(nproc)
```

### 方式 B：手动命令

```bash
cmake -S . -B build/linux-debug-ninja -G Ninja \
      -DINFRA_MODULES='stk;lnx;web' \
      -DINFRA_ENABLE_STK=ON -DINFRA_ENABLE_LNX=ON -DINFRA_ENABLE_WEB=ON

cmake --build build/linux-debug-ninja -j$(nproc)
```

> 说明：虽然模块出现在 `INFRA_MODULES` 中会自动启用，但手动命令中保留
> `-DINFRA_ENABLE_*` 也无害；若你自行改造了 CMake 逻辑，以实际行为为准。

---

## 5. 测试

测试目标通过 CTest 注册，仅当 `-DINFRA_BUILD_TESTS=ON` 时构建。

各模块测试目标：

| 模块 | CTest 测试名 |
|---|---|
| `stk` | `test_stk` |
| `lnx` | `lnx_test_open`、`lnx_test_file_share` |
| `web` | `web_core` |

运行全部测试：

```bash
ctest --test-dir build/linux-debug-ninja --output-on-failure
```

只运行某个模块的测试：

```bash
ctest --test-dir build/linux-debug-ninja -R web_core --output-on-failure
ctest --test-dir build/linux-debug-ninja -R lnx   --output-on-failure
```

构建并使用 AddressSanitizer 跑测试：

```bash
cmake --preset linux-asan-ninja -DINFRA_MODULES='stk;lnx;web'
cmake --build --preset linux-asan-ninja -j$(nproc)
ctest --test-dir build/linux-asan-ninja --output-on-failure
```

---

## 6. 安装与外部消费

安装到系统前缀：

```bash
cmake --install build/linux-debug-ninja --prefix /usr/local
```

外部项目通过 `find_package` 使用（共享库需注意运行时库搜索路径）：

```cmake
find_package(Infra CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE infra::web)
```

---

## 7. 常见问题

**Q：`INFRA_MODULES not specified - no modules will be built.` 然后什么都没编出来？**
A：这是预期提示。构建前必须通过 `-DINFRA_MODULES='stk;lnx;web'` 指定模块。

**Q：配置了但 Enabled Modules 显示 `none`？**
A：同样是没有传 `INFRA_MODULES`。补上即可。

**Q：开启 `-DINFRA_BUILD_DOCS=ON` 但文档没生成？**
A：未安装 Doxygen 时会安全跳过文档生成，不会报错。

**Q：Windows 下能编译吗？**
A：导出宏（`STK_API`/`LNX_API`/`WEB_API`）已支持 Windows DLL 的
`__declspec(dllexport/dllimport)`；但 `lnx` 为 Linux-only，`web` 的 Windows
适配（Winsock 等）尚未完成，目前 Windows 全量构建仍在进行中。

**Q：为什么不用 `GenerateExportHeader` / 不生成 `infra_xxx_export.h`？**
A：项目采用手写 API 宏 + CMake 注入编译定义（`*_DLL` / `*_EXPORTING`）的方式驱动
导出，避免在源码树中生成额外的导出头文件，同时保留跨平台导出能力。

---

## 8. 项目结构（简）

```text
Infrastructure/
├── CMakeLists.txt          # 顶层入口
├── CMakePresets.json        # 构建预设
├── cmake/                   # 构建系统模块（Infra*.cmake）
├── modules/
│   ├── stk/                 # 数据结构/容器
│   ├── lnx/                 # Linux 系统接口
│   └── web/                 # HTTP/WebSocket
├── docs/                    # Doxygen 模板
└── scripts/                 # 辅助脚本
```

---

## 许可证

见 `LICENSE`。
