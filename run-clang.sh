#!/bin/bash

# =====================================
# Senior-level C++ formatting + linting
# =====================================

set -euo pipefail  # Exit on errors, undefined variables, and errors in pipelines

# -------------------------------
# Configuration
# -------------------------------
EXCLUDE="./external|./build|./common/Persistence/tests|./Frontend/tests"
PARALLEL=4                   # Number of parallel clang-tidy processes
CLANG_TIDY_LOG="clang-tidy.log"

#echo "========================================"
#echo "Running clang-format + clang-tidy"
#echo "========================================"

# -------------------------------
# clang-format
# -------------------------------
echo "Running clang-format..."
find . -type f \( -name "*.cpp" -o -name "*.h" \) \
  ! -regex ".*/\(\($EXCLUDE\)/.*\)" \
  -print0 | xargs -0 clang-format -i
echo "clang-format completed."
echo

## -------------------------------
## clang-tidy
## -------------------------------
#echo "Running clang-tidy..."
## Clear previous log
#: > "$CLANG_TIDY_LOG"

#find . -type f -name "*.cpp" ! -regex ".*/\(\($EXCLUDE\)/.*\)" -print0 | \
#  xargs -0 -n 1 -P "$PARALLEL" clang-tidy -fix -p build -- -std=c++20 2>&1 | tee -a "$CLANG_TIDY_LOG"

#echo
#echo "clang-tidy completed. Results saved in $CLANG_TIDY_LOG"
#echo

## -------------------------------
## Check for warnings/errors
## -------------------------------
#if grep -E "warning|error" "$CLANG_TIDY_LOG"; then
#  echo
#  echo "clang-tidy found warnings/errors!"
#  exit 1
#fi

#echo "All formatting and linting completed successfully."
