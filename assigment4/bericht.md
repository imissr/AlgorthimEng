
## Observations and Explanation (simple English)

- **Both parallel algorithms are much faster than `std::sort`.**  
  This is especially true for large arrays.

- **`__gnu_parallel::sort` is always faster than `min_max_quicksort`.**  
  In our measurements:
  - `__gnu_parallel::sort` reaches about **15–16×** speedup compared to `std::sort`.
  - `min_max_quicksort` stays around **11–12×** faster than `std::sort`.

- **With a fixed number of threads (e.g., 24 threads):**  
  - When we **increase the array size**, the speedup first becomes larger.  
  - Reason: the overhead of starting and managing threads is spread over many elements.  
  - After some point, the speedup grows less, because the **memory bandwidth** (RAM) becomes the bottleneck.

- **With a fixed large array size (e.g., N = 30,000,000) and more threads:**
  - At the beginning, performance grows **almost linearly**:  
    2 threads ≈ 2× faster, 4 threads ≈ 4× faster, etc.  
  - After a certain number of threads, the extra benefit becomes smaller because:
    - Some parts of the program **cannot be parallelized** (Amdahl’s Law).
    - There is **overhead for synchronization and management** (coordination between threads, task scheduling).
    - More threads share the same **memory bandwidth**, so they interfere with each other.
  - Because of this, the curve flattens: more threads give only a small additional speedup.

- **Strange spikes or drops (like the dip at 19 threads):**
  - Such outliers are probably due to **measurement noise or the operating system**, for example:
    - The scheduler puts threads on cores in an unlucky way.
    - Background processes briefly use CPU or memory.  

