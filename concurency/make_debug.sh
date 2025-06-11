 time -p cmake  -S./\
 -B./build/debug \
 -D CMAKE_CXX_COMPILER=g++\
 -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Debug .
cd build/debug/
time -p make  -j8