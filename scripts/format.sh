#!/usr/bin/env bash
# [file name]: format.sh
# [file content begin]
# 对项目内所有 C/C++ 源文件执行 clang-format。
#
# 用法：
#   scripts/format.sh          # 就地格式化（-i）
#   scripts/format.sh --check  # 仅检查，不修改（CI 用，发现差异退出码非 0）
#
# 依赖：clang-format（建议 14+，与 CI 保持一致）

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CHECK_ONLY=0

if [[ "${1:-}" == "--check" ]]; then
    CHECK_ONLY=1
fi

# 收集所有 C/C++ 源文件（排除构建目录）
mapfile -t FILES < <(find "${ROOT_DIR}/modules" \
    -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.hpp' -o -name '*.tcc' \) \
    -not -path '*/build/*')

if [[ ${#FILES[@]} -eq 0 ]]; then
    echo "[format] no source files found"
    exit 0
fi

if [[ ${CHECK_ONLY} -eq 1 ]]; then
    echo "[format] checking ${#FILES[@]} files ..."
    # --dry-run --Werror：有任一文件不符合格式则非零退出
    clang-format --dry-run --Werror "${FILES[@]}"
    echo "[format] OK"
else
    echo "[format] formatting ${#FILES[@]} files ..."
    clang-format -i "${FILES[@]}"
    echo "[format] done"
fi
