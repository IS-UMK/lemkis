# Configuration

You may need to install ``openmpi openmpi-devel``.

and update paths
```bash
 export PATH=/usr/lib64/mpi/gcc/openmpi5/bin:$PATH
 export LD_LIBRARY_PATH=/usr/lib64/mpi/gcc/openmpi5/lib64:$LD_LIBRARY_PATH
```

# building

just create `build` director `mkdir build` go to it `cd build` and execute
```bash
CXX=mpicxx cmake .. && make
```
