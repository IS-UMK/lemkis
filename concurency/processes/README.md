# Basic notions

## Virtual memory

**Virtual memory** is a memory management technique used by modern operating systems to give applications the illusion that they have access to a large, 
continuous block of memory, even if the physical RAM is limited or fragmented. It achieves this 
by using both the computer's physical memory (RAM) and a portion of secondary storage (such as a hard drive or SSD) as an extension of RAM


## Memory page

**page** (also called a **memory page** or **virtual page**) is a fixed-length, contiguous block of virtual memory that serves as the smallest unit of data for memory management in operating systems that use virtual memory. Each page is described by a single entry in a page table, which maps virtual pages to physical memory locations (called page frames).

The operating system divides memory into these pages to efficiently manage, allocate, and move data. When a program needs more memory than is physically available, the OS can transfer pages between main memory (RAM) and secondary storage (such as a hard disk) in a process called paging or swapping.

Pages are typically 4 KB or 8 KB in size, but the exact size depends on the system architecture. The use of pages allows the OS to provide each process with its own continuous virtual address space, even if the underlying physical memory is fragmented or limited.

## Shared memory

**Shared memory** is a fundamental inter-process communication (IPC) mechanism that enables multiple processes to access the same region of memory, allowing them to exchange data efficiently without redundant copies or complex message-passing protocols. Here's a structured breakdown:

### Core Characteristics

1. Simultaneous Access: Multiple processes can read/write the same memory region concurrently.
2. Efficiency: Avoids data copying between processes (unlike pipes/sockets).
3. Persistence: Survives process termination until explicitly deleted (shm_unlink).
4. Virtual Filesystem: On Linux, POSIX shared memory objects reside in /dev/shm (a tmpfs RAM disk).


### Key Advantages
1. Performance: Direct memory access eliminates serialization/deserialization overhead.
2. Atomic Operations: Processes can use synchronization primitives (semaphores) with shared memory.
3. Large Data Handling: Suitable for sharing substantial datasets between process

### Use Cases
1. High-Performance Computing:
2. Real-time data processing pipelines.
3. Multi-processor task coordination.
4. Database Systems: Shared query caches.
5. Gaming/Graphics: Texture/asset sharing between processes.

### Critical Considerations
1. Synchronization: Requires mutexes/semaphores to prevent race conditions.
2. Security: Permissions (0600 in shm_open) control access.
3. Resource Management: Always munmap and shm_unlink to prevent memory leaks. Use ftruncate cautiously to avoid oversized allocations

## POSIX Shared Memory Functions  

The core functions for POSIX shared memory management form a workflow for creating, accessing, and cleaning up shared memory regions. Here's the essential toolkit:

### 1. `int shm_open(const char *name, int oflag, mode_t mode);`  
**Purpose**: Creates/opens a shared memory object (persistent or temporary).  
**Params** 
```cpp
const char *name: "/my_shm" // Must begin with '/'
int oflag: O_CREAT | O_RDWR // Common flags
mode_t mode: 0600 // User read/write permissions
```
**Remainder**
`shm_open` returns a file descriptor because POSIX shared memory objects are designed to integrate seamlessly with the existing UNIX file and memory management infrastructure. In UNIX-like systems, a file descriptor is a universal handle that represents an open file, device, pipe, or other resources—including shared memory objects. This design choice offers several advantages:

1. **Uniformity**: By representing shared memory as a file descriptor, the same system calls and APIs (like mmap, ftruncate, close) used for regular files can also be used for shared memory. This simplifies the API and makes it easier for developers to manage different types of resources in a consistent way.
2. **Resource Management**: File descriptors provide a well-understood mechanism for tracking resource usage, permissions, and cleanup. When a shared memory object is opened, the kernel creates an "open file description" and assigns a file descriptor to it, just like with files.
3. **Interoperability**: Many system calls that operate on files (such as mmap for mapping memory) require a file descriptor as an argument. By returning a file descriptor, shm_open allows shared memory objects to be mapped into a process's address space using the same mechanisms as files.
4. **Precedent**: In POSIX and UNIX, file descriptors are already used for various object types, including regular files, pipes, sockets, and devices. Extending this model to shared memory objects keeps the system interface simple and extensible.

**Behavior**:  
- Returns file descriptor for memory object  
- Analogous to `open()` for files  
- Creates persistent object if `O_CREAT` specified  

### 2. `shm_unlink("/my_shm"); // Immediate name removal`
**Purpose**: Removes shared memory object name  


## Complementary System Calls
### 3. `int ftruncate(int fd, off_t length);`
**Purpose**: Sets shared memory size  
**Usage**:  
```cpp
ftruncate(shm_fd, 4096); // Set 4KB size
// Required after creation to allocate space
```
### 4. `void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);`
**Purpose**: Maps memory into address space  
**Typical Usage**:
```cpp
char *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
// MAP_SHARED ensures cross-process visibility
```
### 5. `int munmap(void *addr, size_t length);`
**Purpose**: Unmaps memory region  
**Usage**:  
```cpp
munmap(ptr, 4096); // Release virtual mapping
// Doesn't affect other process mappings
```


### 6. `int close(int fd);`
**Purpose**: Releases file descriptor  
**Signature**:  

## Complete Workflow Example

sequenceDiagram
Process->>OS: shm_open("/mem", O_CREAT|O_RDWR, 0600)
OS-->>Process: fd=3
Process->>OS: ftruncate(3, 4096)
Process->>OS: mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, 3, 0)
OS-->>Process: ptr=0x7f1234567890
Process->>OS: shm_unlink("/mem")
Process->>OS: munmap(0x7f1234567890, 4096)
Process->>OS: close(3)


**Typical Initialization Sequence**:  
```cpp
// 1. Create shared memory
int fd = shm_open("/shm", O_CREAT|O_RDWR, 0600);

// 2. Allocate size
ftruncate(fd, 4096);

// 3. Map memory
void *ptr = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

// 4. Cleanup name
shm_unlink("/shm");

// 5. Release descriptor
close(fd);

// ... Use memory ...

// 6. Unmap when done
munmap(ptr, 4096);
```

**Cleanup Order**:  
1. `munmap()` all memory regions  
2. `close()` associated file descriptors  
3. `shm_unlink()` names (preferably early)  

**Key Features**:  
- **Persistence**: Objects survive process termination unless unlinked  
- **Security**: Permissions controlled via `mode` parameter  
- **Efficiency**: Shared memory provides fastest IPC mechanism  
- **Atomicity**: OS guarantees full page transfers  

> **Pro Tip**: Always check return values for errors in production code. Use `perror()` or `strerror(errno)` for debugging.  
> **Warning**: Never store pointers in shared memory - virtual addresses differ between processes. Use offsets instead.

### Error handling

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

extern int sys_nerr;

void syserr(const char *fmt, ...)
{
  va_list fmt_args;                // 1. Declare argument list holder
  fprintf(stderr, "ERROR: ");      // 2. Print standard error prefix
  va_start(fmt_args, fmt);         // 3. Initialize argument processing
  vfprintf(stderr, fmt, fmt_args); // 4. Print formatted message with args
  va_end(fmt_args);                // 5. Clean up argument list
  fprintf(stderr," (%d; %s)\n",    // 6. Append system error details
          errno, strerror(errno)); //    - errno: Last error number
                                   //    - strerror: Human-readable message
  exit(1);                         // 7. Terminate program with error code
}

// Example usage: if (open("file.txt", O_RDONLY) == -1) syserr("File open failed");
```
