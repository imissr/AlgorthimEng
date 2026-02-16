## 1. Selected Lecture Slide and Explanation

### 1.1 Chosen Slide

I refer to the lecture slide **“Why Parallel Computing? – The Free Lunch Is Over.”**

### 1.2 Content of the Slide

- In the past, processors became continuously faster, mainly due to higher **clock frequencies** and improved **microarchitectures**.
- Programmers did not need to change their code:  
  Sequential programs automatically ran faster on newer hardware.  
  → This phenomenon is known as the **free lunch**.
- Due to physical limits (power consumption, heat dissipation), clock frequencies can no longer be increased arbitrarily.
- Instead, modern processors provide:
  - more **cores per chip**, and
  - wider **SIMD/vector units**.

### 1.3 Why This Is Important

The statement **“The free lunch is over”** means:

- Sequential programs no longer become faster automatically on modern processors.
- To achieve higher performance, programmers must **explicitly exploit parallelism**, for example by using:
  - multiple threads or processes,
  - OpenMP, tasks, etc.,
  - vectorized computations,
  - cache-friendly memory access patterns.

---

## 2. Two Interesting Points from Chapter 1 of *Computer Systems: A Programmer’s Perspective*

### 2.1 “Information Is Bits + Context”

The chapter emphasizes that a computer ultimately operates only on **bit patterns**:

- Hardware sees only sequences of 0s and 1s.
- Whether a particular bit pattern represents
  - an integer,
  - a floating-point number,
  - a character,
  - or a machine instruction  
    depends entirely on the **context** in which it is interpreted.

**Why I find this interesting:**

- It explains many common programming errors:
  - integer overflow,
  - unexpected values when reading memory with the wrong type,
  - problems with casts and pointers.
- The computer does not know what is “meaningful” or “wrong”; it only applies rules to bits.
- This understanding is essential for working correctly with:
  - data representations,
  - pointers,
  - machine-level code.

**In short:**

> I found the statement “Information is bits + context” particularly interesting. The hardware only knows bit patterns; whether they represent an integer, a float, or a character depends entirely on how the program interprets them. This explains why many errors do not produce immediate error messages but instead result in nonsensical values.

---

### 2.2 “Caches Matter”

- Programs spend a significant amount of time **moving data** (disk → RAM → CPU).
- **Large = slow**, **small = fast and expensive** → the processor–memory gap.
- **Caches (L1, L2, L3)** are small, fast memories between the CPU and main memory.
- They exploit **locality**:
  - frequently used data and nearby memory locations stay in the cache,
  - resulting in faster access.
- Important insight for me: performance strongly depends on **data layout and access patterns**, not just on the number of operations.

---

## 3. Parallelization of the Monte Carlo π Program (OpenMP)

### 3.3 Example Implementation (C++ with OpenMP)

```cpp
#include <iostream>
#include <iomanip>
#include <random>
#include <omp.h>

using namespace std;

int main() {
    int n = 100000000;          // number of random points
    int counter = 0;            // points inside the quarter circle

    double start_time = omp_get_wtime();

    // Parallel region
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        // Each thread gets its own random number generator
        unsigned int seed = 1234u + tid;
        default_random_engine re(seed);
        uniform_real_distribution<double> zero_to_one(0.0, 1.0);

        int local_counter = 0;  // local counter per thread

        // Loop is distributed across threads
        #pragma omp for
        for (int i = 0; i < n; ++i) {
            double x = zero_to_one(re);
            double y = zero_to_one(re);

            if (x * x + y * y <= 1.0) {
                ++local_counter;
            }
        }

        // Safe accumulation into the global counter
        #pragma omp atomic
        counter += local_counter;
    }

    double run_time = omp_get_wtime() - start_time;
    double pi = 4.0 * static_cast<double>(counter) / static_cast<double>(n);

    cout << "pi: "       << setprecision(17) << pi       << "\n";
    cout << "run_time: " << setprecision(6)  << run_time << " s\n";
    cout << "n: "        << n << "\n";
}
