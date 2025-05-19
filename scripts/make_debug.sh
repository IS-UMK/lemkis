CXX=clang++-19 CC=clang-19 cmake -B./build/debug -GNinja \
 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON\
 -DCMAKE_BUILD_TYPE=Debug\
 . \
&& cd build/debug && ninja 
