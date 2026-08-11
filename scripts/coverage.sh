#!/usr/bin/env bash
# [file name]: coverage.sh
# [file content begin]
# 构建并运行测试，然后生成覆盖率报告。
#
# 用法：
#   scripts/coverage.sh [build_dir]
#
# 依赖：cmake、gcov；HTML 报告可选 gcovr。

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build/coverage}"

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DINFRA_LIBRARY_TYPE=STATIC \
    -DINFRA_BUILD_TESTS=ON \
    -DINFRA_ENABLE_COVERAGE=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "${BUILD_DIR}"
ctest --test-dir "${BUILD_DIR}" --output-on-failure

if command -v gcovr >/dev/null 2>&1; then
    REPORT_DIR="${BUILD_DIR}/coverage"
    mkdir -p "${REPORT_DIR}"
    gcovr "${BUILD_DIR}" \
        --root "${ROOT_DIR}" \
        --filter "${ROOT_DIR}/modules/" \
        --exclude ".*/tests/.*" \
        --html-details "${REPORT_DIR}/index.html" \
        --print-summary
    echo "[coverage] HTML report: ${REPORT_DIR}/index.html"
else
    echo "[coverage] tests passed; install gcovr to generate HTML details"
fi
