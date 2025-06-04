# Summary: CUDA Kernel Basics and Matrix Multiplication Example

This summary covers the essentials of writing CUDA kernels, how threads, blocks, and grids are organized, and a simple matrix multiplication kernel with host code example.

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
global void matMulKernel(const float* A, const float* B, float* C,
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

```


---

## Example Host Code to Use the Kernel

```cpp
#include <cuda_runtime.h>
#include <iostream>
#include <cmath>

#define CUDA_CHECK(call)
do {
cudaError_t err = call;
if (err != cudaSuccess) {
std::cerr << "CUDA error: " << cudaGetErrorString(err) << std::endl;
exit(EXIT_FAILURE);
}
} while(0)

int main() {
int M = 512, K = 256, N = 512;
size_t sizeA = M * K * sizeof(float);
size_t sizeB = K * N * sizeof(float);
size_t sizeC = M * N * sizeof(float);
float *h_A = new float[M * K];
float *h_B = new float[K * N];
float *h_C = new float[M * N];

// Initialize matrices with 1.0f for simplicity
for (int i = 0; i < M * K; ++i) h_A[i] = 1.0f;
for (int i = 0; i < K * N; ++i) h_B[i] = 1.0f;

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
std::cout << (success ? "Matrix multiplication succeeded!" : "Result verification failed!") << std::endl;

CUDA_CHECK(cudaFree(d_A));
CUDA_CHECK(cudaFree(d_B));
CUDA_CHECK(cudaFree(d_C));

delete[] h_A;
delete[] h_B;
delete[] h_C;

return 0;
```


---

## Key Points Recap

- Use **grids of blocks** to cover large matrices; each thread computes one output element.
- Compute **global thread indices** using `blockIdx`, `blockDim`, and `threadIdx`.
- Always do **boundary checks** inside kernels.
- Manage GPU memory carefully: allocate, copy, launch kernel, copy back, free.
- The indexing formula stays the same whether you use one block or multiple blocks (grid).
- For better performance, consider **tiling and shared memory**, but start with this simple version.

---

## References

- [NVIDIA CUDA Samples: matrixMul](https://github.com/NVIDIA/cuda-samples/blob/master/Samples/0_Introduction/matrixMul/matrixMul.cu)
- CUDA C Programming Guide (NVIDIA)
- CUDA tutorials and articles on matrix multiplication

---

This should give you a solid foundation to write and run your own CUDA kernels for matrix multiplication and other parallel tasks!

