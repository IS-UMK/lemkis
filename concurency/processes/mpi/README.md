# Configuration

You may need to install ``openmpi openmpi-devel``. in my case i had to install manually [version 5](https://www.open-mpi.org/software/ompi/v5.0/) with configuration:
```bash
CC=clang-19 CXX=clang++-19 ./configure --prefix=$HOME/opt/openmpi --disable-mpi-fortran
```

and update paths
```bash
 export PATH=/usr/lib64/mpi/gcc/openmpi5/bin:$PATH
 export LD_LIBRARY_PATH=/usr/lib64/mpi/gcc/openmpi5/lib64:$LD_LIBRARY_PATH
```

# local building
If you want to build locally (not whole lemkis) do this:
create `build` director `mkdir build` go to it `cd build` and execute
```bash
CXX=mpicxx cmake .. && make
```

# running
```bash
mpirun -n <number_of_processes_to_launch> <program>
```
