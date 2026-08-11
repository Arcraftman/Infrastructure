#!/usr/bin/env bash
# [file name]: tidy.sh
# [file content begin]
# 对整个项目运行 clang-tidy 静态分析。
#
# 用法：
#   scripts/tidy.sh [build_dir]
#
# 需要一个已 configure 的构建目录（含 compile_commands.json）。
# 默认使用 build/linux-debug-ninja（CMakePresets.json 中已开启
# CMAKE_EXPORT_COMPILE_COMMANDS=ON）。
#
# 依赖：clang-tidy（或 run-clang-tidy 并行版本）

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build/linux-debug-ninja}"
COMPDB="${BUILD_DIR}/compile_commands.json"

if [[ ! -f "${COMPDB}" ]]; then
    echo "[tidy] compile_commands.json not found at: ${COMPDB}" >&2
    echo "[tidy] run: cmake --preset linux-debug-ninja first" >&2
    exit 1
fi

# 优先用 run-clang-tidy（并行）；否则退化为逐文件 clang-tidy
if command -v run-clang-tidy >/dev/null 2>&1; then
    echo "[tidy] running run-clang-tidy (parallel) ..."
    run-clang-tidy -p "${BUILD_DIR}" -quiet
else
    echo "[tidy] run-clang-tidy not found, falling back to clang-tidy ..."
    mapfile -t FILES < <(find "${ROOT_DIR}/modules" \
        -type f \( -name '*.c' -o -name '*.cpp' \) \
        -not -path '*/build/*')
    clang-tidy -p "${BUILD_DIR}" "${FILES[@]}"
fi

echo "[tidy] done"
