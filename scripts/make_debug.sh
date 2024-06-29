CXX=clang++ CC=clang cmake -B./build/debug -GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug . && cd build/debug && ninja 
