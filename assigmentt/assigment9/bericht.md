# How bandwidth bound computations differ from compute bound computations

A compute bound computation is limited by the CPU.
This means the program spends most of its time doing arithmetic or logic operations.
Even if memory became faster, the program would not speed up much because the CPU work is the bottleneck.
The lecture gives an example of estimating pi with numerical integration, where each loop step does mostly math. 

A bandwidth bound computation is limited by memory.
This means the program spends most of its time waiting for data to be read from memory, especially from RAM.
Even if the CPU is very fast, the program cannot run faster if data does not arrive fast enough.
The lecture gives an example of summing a large matrix stored in RAM, where reading the elements is the bottleneck. 

The lecture also notes that cache optimization is especially important for bandwidth bound computations because their performance depends heavily on memory behavior. 

# Why temporal locality and spatial locality improve program performance

Caches transfer data in blocks called cache lines.
A cache line is the smallest block that is moved between memory and cache.
The lecture mentions that on Intel a cache line is 64 bytes. 

Temporal locality means that if a program uses some data once, it is likely to use the same data again soon.
If the data is reused quickly, it is still in cache, so the next access is fast.
This improves performance because it avoids slow main memory access. 

Spatial locality means that if a program accesses one memory location, it is likely to access nearby locations soon.
Because a whole cache line is loaded, nearby values may already be in cache.
So sequential access is fast, while jumping around in memory is slow.

The lecture shows this idea using two matrix summation loops.
Row major order access has good spatial locality.
Column wise access has poor spatial locality because it jumps with a large stride. 

In simple words
Temporal locality helps because you reuse the same data while it is still in fast cache.
Spatial locality helps because when you load one value you also load nearby values for free in the same cache line. 


# Report on One Lecture Slide: Cache-Oblivious Algorithms

**Selected slide:** **“Side Note: Cache-Oblivious Algorithms”** (Slide/Page **19 of 21**)


## 1. What the slide says (summary)

The slide introduces **cache-oblivious algorithms** as algorithms that:

- exploit CPU caches **without** using cache parameters (cache size, cache-line length, etc.) as explicit inputs,
- aim for **asymptotically minimal cache misses**, independent of the cache hierarchy,
- contrast with **explicit blocking** (a.k.a. *cache-aware tiling*), where you choose block sizes tuned to a specific machine,
- are often implemented via **recursive divide-and-conquer**,
- may or may not be faster in practice and can be more complex to implement.



# Report on One Lecture Slide: Cache-Oblivious Algorithms

**Selected slide:** **“Side Note: Cache-Oblivious Algorithms”** (Slide/Page **19 of 21**)

Cache-oblivious algorithms are designed to use caches well **without knowing cache parameters** (like cache size or line size). They aim for **few cache misses** across the whole cache hierarchy, often using **recursive divide-and-conquer**. They differ from **cache-aware blocking**, where block sizes are tuned for a specific machine.

## Cache-aware vs. Cache-oblivious (short)
- **Cache-aware (blocking):** pick a tile size `b` based on hardware.  
  **Pros:** can be very fast when tuned. **Cons:** not portable; hard across multiple cache levels.
- **Cache-oblivious:** no `(M, B)` parameters; recursion creates subproblems that eventually fit in cache.  
  **Pros:** portable; works across cache levels. **Cons:** more complex; performance depends on constants/prefetching/TLB.

## Why divide-and-conquer helps
Recursion shrinks the working set step by step. Once a subproblem fits into some cache level, the algorithm gains:
- **temporal locality** (reuse),
- **spatial locality** (nearby access),
  which reduces cache misses.



# Two interesting points from the paper

## 1) Copying tiles to avoid cache conflicts
Even with blocking/tiling, you can still get lots of **conflict misses** (self-interference) because parts of a tile may map to the same cache sets. The paper notes that **copying a block into a contiguous buffer** can reduce these conflicts and improve performance, even though copying adds extra work. :contentReference[oaicite:0]{index=0}

## 2) ATLAS: performance by empirical tuning + multi-level blocking
ATLAS generates many BLAS variants (unrolling, register blocking, cache blocking) and **benchmarks** them to pick the best for the machine. The paper highlights how **matrix multiply** is engineered with **L1 micro-kernels** and additional blocking for higher cache levels (L2/L3) to maximize reuse. :contentReference[oaicite:1]{index=1}
