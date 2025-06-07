# Format all source and header files in-place using clang-format-19
find ./src -iname '*.cxx' -o -iname '*.ixx' -o -iname '*.cpp' -o -iname '*.hpp' | xargs clang-format-19 -i

# Run clang-format-19 in dry-run mode with warnings treated as errors, to check formatting without modifying files
find ./src -iname '*.cxx' -o -iname '*.ixx' -o -iname '*.cpp' -o -iname '*.hpp' | xargs clang-format-19 -i --Werror --dry-run

# Run clang-tidy-19 with fixes applied automatically based on the .clang-tidy config and compile commands
find ./src -iname '*.cxx' -o -iname '*.ixx' -o -iname '*.cpp' -o -iname '*.hpp' | xargs clang-tidy-19 -p ./build/ --extra-arg=-std=c++26 --config-file=./.clang-tidy --fix --fix-errors
