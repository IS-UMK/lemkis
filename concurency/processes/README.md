# Basic notions


**Virtual memory** is a memory management technique used by modern operating systems to give applications the illusion that they have access to a large, 
continuous block of memory, even if the physical RAM is limited or fragmented. It achieves this 
by using both the computer's physical memory (RAM) and a portion of secondary storage (such as a hard drive or SSD) as an extension of RAM


**page** (also called a **memory page** or **virtual page**) is a fixed-length, contiguous block of virtual memory that serves as the smallest unit of data for memory management in operating systems that use virtual memory. Each page is described by a single entry in a page table, which maps virtual pages to physical memory locations (called page frames).

The operating system divides memory into these pages to efficiently manage, allocate, and move data. When a program needs more memory than is physically available, the OS can transfer pages between main memory (RAM) and secondary storage (such as a hard disk) in a process called paging or swapping.

Pages are typically 4 KB or 8 KB in size, but the exact size depends on the system architecture. The use of pages allows the OS to provide each process with its own continuous virtual address space, even if the underlying physical memory is fragmented or limited.
