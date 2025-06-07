find ./src -iname '*.cxx' -o -iname '*.ixx' -o -iname '*.cpp'  -o -iname '*.hpp' | xargs clang-format-19 -i
find ./src -iname '*.cxx' -o -iname '*.ixx' -o -iname '*.cpp'  -o -iname '*.hpp' | xargs clang-format-19 -i --Werror --dry-run

find ./src -iname '*.cxx' -o -iname '*.ixx' -o -iname '*.cpp'  -o -iname '*.hpp' | xargs clang-tidy-19  -p ./build/ --extra-arg=-std=c++26 --config-file=./.clang-tidy
