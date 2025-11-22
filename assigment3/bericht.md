## When run with one thread per logical core, threads from Listing 3.1 display their numbers randomly, while those from Listing 3.2 show them in the same order. Explain why.

### Listing 3.1 – “Hello world”

```c
#pragma omp parallel
printf(" %d", omp_get_thread_num());
```

- All threads are created, and **each one immediately executes a single `printf`**.
- There is **no extra computation** that would make one thread noticeably slower or faster than another.
- The **OS / OpenMP runtime schedules these threads independently and arbitrarily** on the logical cores.
- They all **race to write to the same standard output stream**; the runtime just serializes the access internally, but **the order in which threads reach `printf` is essentially random**.

**So you see the thread numbers as a random permutation.**


---

### Listing 3.2 – Fibonacci

```c
#pragma omp parallel
{
    int t = omp_get_thread_num();
    printf("%d: %ld\n", t, fib(n + t));
}
```

#### Key differences

- **Each thread does a lot of work before printing.**
    - Every thread calls `fib(n + t)` with a naive recursive implementation.
    - The runtime of this `fib` grows *exponentially* with its argument:  
      `time(fib(n+1)) >> time(fib(n))`, etc.

- **The work per thread is systematically ordered by `t`.**

---

#### Result

- Threads **finish** (and thus reach `printf`) roughly in order of their workload, i.e. in order of `t`.
- So their outputs appear in **the same order every time**:  
  **thread 0’s line first, then thread 1, and so on.**

## What do you think, does the code snippet in Listing 3.10 use the cores efficiently, why yes, why no. Consider also the variable size in your argumentation.


- The code uses the cores **well when `size` is large**:  
  With `collapse(2)` there are `size²` independent iterations that can be evenly distributed across threads.

- It is **inefficient when `size` is small**:  
  Then `size²` is not much larger than the number of threads, so the overhead of starting the parallel region in every generation is high compared to the work, and some cores may be almost idle.

- It would be more efficient to **create the `#pragma omp parallel` region once** outside the `while (gens--)` loop and use only `#pragma omp for collapse(2)` inside.




