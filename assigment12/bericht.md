# Report on Three Slides (with extra research)

> Source: Lecture PDF **12_Advanced_Topics.pdf**

---

## 1) Parallelization: Dynamic Parallel Computations With Divide and Conquer (page 9)

### What the slide shows
A **divide-and-conquer parallel loop** using `std::async(std::launch::async, ...)` to denoise images in parallel.  
The work is split recursively until `n <= chunk_sz`, then processed sequentially.

### Deeper research & key points
- **`std::async` is not a thread pool.** With `std::launch::async`, tasks run asynchronously, but the implementation may create threads aggressively. behavior is implementation-specific.
- **Task granularity matters.** `chunk_sz` limits recursion and prevents spawning too many tiny tasks. If too small, overhead can dominate.
- **Oversubscription risk.** Recursive `async` can create more runnable tasks than CPU cores, leading to context switching and memory overhead.

### Practical recommendations
- Choose `chunk_sz` so each task does **enough work** (often milliseconds, not microseconds).
- Consider a **thread pool or task scheduler** for many short tasks or repeated runs.
- Always profile on the target system.

---

## 2) SIMD: Architecture-Independent SIMD With Google Highway (page 5)

### What the slide shows
Using **Google Highway** for portable SIMD:
- aligned arrays (`HWY_ALIGN`)
- `ScalableTag<DT>`
- `Lanes(d)` to get vector width
- vectorized load/add/store inside a loop

### Deeper research & key points
- The main goal of Highway is **performance-portable SIMD**, allowing the same code to run efficiently on different CPU architectures (x86, ARM).
- **Length-agnostic vectors** avoid hardcoding SIMD widths (e.g., AVX2 vs AVX-512).
- This approach reduces the need for handwritten intrinsics while keeping good performance.

### Practical notes / pitfalls
- Correct **alignment** is important for performance.
- **Remainder handling** is required when array size is not a multiple of SIMD lane count.
- Compilation flags and target architecture still influence performance.

---

## 3) NUMA: Exploit First-Touch Data Placement (page 16)

### What the slide shows
A NUMA-aware memory strategy:
- allocate memory using `malloc`
- initialize the array **in parallel** using `#pragma omp parallel for schedule(static)`
- use the same static scheduling during computation

### Deeper research & key points
- Many operating systems use a **first-touch policy**, where memory pages are placed on the NUMA node of the CPU that first accesses them.
- Parallel initialization ensures that each thread gets memory located close to its CPU.
- NUMA memory policies can further control placement when first-touch is insufficient.

### Why `calloc` or naive `vector` initialization is discouraged
- Serial initialization can cause all pages to be placed on a single NUMA node.
- This leads to expensive remote memory access for other threads.

### Practical checklist
- Keep **thread affinity stable** (pin threads if possible).
- Use **static scheduling** when data ownership is fixed per thread.
- For irregular access patterns, consider NUMA interleaving or explicit placement tools.

---
