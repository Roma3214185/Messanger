#!/bin/bash

# Format everything
echo "Running clang-format..."
EXCLUDE="./external"
find . -type f \( -name "*.cpp" -o -name "*.h" \) \
  ! -path "$EXCLUDE/*" \
  -print0 | xargs -0 clang-format -i

# Run clang-tidy
echo "Running clang-tidy..."
find . -name "*.cpp" ! -path "$EXCLUDE/*" -print0 | \
  xargs -0 -n 1 -P 4 clang-tidy -fix -p build -- -std=c++20

echo "Done!"
