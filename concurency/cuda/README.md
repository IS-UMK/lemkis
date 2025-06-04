# CUDA Kernel Basics and Matrix Multiplication Example


---

## CUDA Thread Hierarchy

- **Thread:** Smallest execution unit running kernel code.
- **Block:** Group of threads that can cooperate via shared memory and synchronize.
- **Grid:** Collection of blocks launched together to cover large data.

Threads are indexed within blocks by `threadIdx`, blocks within grids by `blockIdx`, and block/grid dimensions by `blockDim` and `gridDim`.

Global thread indices for 2D problems (e.g., matrices) are computed as:

```cpp
int row = blockIdx.y * blockDim.y + threadIdx.y;
int col = blockIdx.x * blockDim.x + threadIdx.x;
```


This formula is the same whether you launch one block or many blocks (a grid). Using a grid allows covering large matrices by extending the range of `blockIdx`.

---

## Writing a CUDA Kernel

- Declare kernel functions with `__global__`.
- Compute unique thread indices to map threads to data elements.
- Use boundary checks to avoid out-of-range memory access.
- Launch kernels with `kernel<<<gridDim, blockDim>>>(args);`.
- Manage device memory allocation, copying, and freeing on the host.

---

## Simple Matrix Multiplication CUDA Kernel

```cpp
// Matrix multiplication kernel: C = A * B
// A is M x K, B is K x N, C is M x N
__global__ void matMulKernel(const float* A, const float* B, float* C,
int M, int N, int K) {
int row = blockIdx.y * blockDim.y + threadIdx.y; // Row index of C
int col = blockIdx.x * blockDim.x + threadIdx.x; // Column index of C
if (row < M && col < N) {
    float sum = 0.0f;
    for (int i = 0; i < K; ++i) {
        sum += A[row * K + i] * B[i * N + col];
    }
    C[row * N + col] = sum;
}
}
```


---

## Example Host Code to Use the Kernel

```cpp
#include <cmath>
#include <cuda_runtime.h>
#include <iostream>
// Matrix multiplication kernel: C = A * B
// A is M x K, B is K x N, C is M x N
__global__ void matMulKernel(const float* A, const float* B, float* C,
int M, int N, int K) {
int row = blockIdx.y * blockDim.y + threadIdx.y; // Row index of C
int col = blockIdx.x * blockDim.x + threadIdx.x; // Column index of C
if (row < M && col < N) {
    float sum = 0.0f;
    for (int i = 0; i < K; ++i) {
        sum += A[row * K + i] * B[i * N + col];
    }
    C[row * N + col] = sum;
}
}

#define CUDA_CHECK(call)                                                       \
  do {                                                                         \
    cudaError_t err = call;                                                    \
    if (err != cudaSuccess) {                                                  \
      std::cerr << "CUDA error: " << cudaGetErrorString(err) << std::endl;     \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

int main() {
  int M = 512, K = 256, N = 512;
  size_t sizeA = M * K * sizeof(float);
  size_t sizeB = K * N * sizeof(float);
  size_t sizeC = M * N * sizeof(float);
  float *h_A = new float[M * K];
  float *h_B = new float[K * N];
  float *h_C = new float[M * N];

  // Initialize matrices with 1.0f for simplicity
  for (int i = 0; i < M * K; ++i)
    h_A[i] = 1.0f;
  for (int i = 0; i < K * N; ++i)
    h_B[i] = 1.0f;

  float *d_A, *d_B, *d_C;
  CUDA_CHECK(cudaMalloc(&d_A, sizeA));
  CUDA_CHECK(cudaMalloc(&d_B, sizeB));
  CUDA_CHECK(cudaMalloc(&d_C, sizeC));

  CUDA_CHECK(cudaMemcpy(d_A, h_A, sizeA, cudaMemcpyHostToDevice));
  CUDA_CHECK(cudaMemcpy(d_B, h_B, sizeB, cudaMemcpyHostToDevice));

  dim3 threadsPerBlock(16, 16);
  dim3 blocksPerGrid((N + threadsPerBlock.x - 1) / threadsPerBlock.x,
                     (M + threadsPerBlock.y - 1) / threadsPerBlock.y);

  matMulKernel<<<blocksPerGrid, threadsPerBlock>>>(d_A, d_B, d_C, M, N, K);
  CUDA_CHECK(cudaDeviceSynchronize());

  CUDA_CHECK(cudaMemcpy(h_C, d_C, sizeC, cudaMemcpyDeviceToHost));

  // Verify result: each element should be K (since all ones multiplied)
  bool success = true;
  for (int i = 0; i < M * N; ++i) {
    if (fabs(h_C[i] - K) > 1e-5) {
      success = false;
      std::cerr << "Mismatch at index " << i << ": " << h_C[i] << std::endl;
      break;
    }
  }
  std::cout << (success ? "Matrix multiplication succeeded!"
                        : "Result verification failed!")
            << std::endl;

  CUDA_CHECK(cudaFree(d_A));
  CUDA_CHECK(cudaFree(d_B));
  CUDA_CHECK(cudaFree(d_C));

  delete[] h_A;
  delete[] h_B;
  delete[] h_C;

  return 0;

}
```


# CUDA Function Qualifiers: `__global__`, `__device__`, and `__host__`

In CUDA C++, these qualifiers specify where functions run and from where they can be called.

---

## `__global__`

- Declares a **kernel function**.
- Runs **on the GPU (device)**.
- Called **from the CPU (host)** using CUDA kernel launch syntax:
  ```cpp
  kernel<<<gridDim, blockDim>>>(args);
  ```
- Must have `void` return type.
- Each GPU thread executes an instance of this function in parallel.

**Example:**
```cpp
__global__ void myKernel() {
// kernel code running on GPU
}
```


---

## `__device__`

- Declares a **device function**.
- Runs **on the GPU (device)**.
- Can only be called **from other device or global (`__global__`) functions**.
- Can have any return type.
- Used for helper functions callable from kernels or other device functions.

**Example:**
```cpp
__device__ int add(int a, int b) {
return a + b;
}
```

---

## `__host__`

- Declares a **host function**.
- Runs **on the CPU (host)**.
- Called **from host code only**.
- This is the default for normal C++ functions, so usually not explicitly needed.

**Example:**
```cpp
__host__ void foo() {
// CPU code
}
```


---

## Combined Qualifiers

You can combine `__host__` and `__device__` to compile a function for **both CPU and GPU**:
```cpp
__host__ __device__ int square(int x) {
return x * x;
}
```



