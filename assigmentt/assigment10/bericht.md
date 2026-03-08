## Knuth on “Premature Optimization” (p.16)

Donald Knuth’s message is often shortened to *“premature optimization is the root of all evil,”* but his real point is more balanced. He argues that optimizing code **before knowing where the real bottlenecks are** wastes time and harms readability and maintainability. Instead, programmers should **measure first** and only optimize the **critical few percent** of the code that actually affects performance.

**My opinion:** I agree. Clean and correct code should come first. Optimization should be driven by profiling and real data, not intuition. Early micro-optimizations usually create complexity without real benefit.
There’s also a difference between good design choices (e.g., picking a better algorithm/data structure, reducing asymptotic complexity) and premature micro-optimizations (manual tweaks before you even know they matter). The former is often worth doing early, the latter usually isn’t.

## Selected Slide (not 15–16): PerfEvent (p.17)

PerfEvent is a lightweight C++ wrapper around Linux’s `perf_event` API, which allows developers to measure **hardware performance counters** such as CPU cycles, instructions, cache misses, and branch misses for specific code sections.
Why this is powerful for algorithm engineering:

It lets you go beyond “runtime in seconds” and ask why something is slow:

Are you limited by memory bandwidth (lots of cache misses)?

Are you limited by CPU execution (high cycles per instruction)?

Is branching unpredictable (branch misses)?

This connects directly to the main idea: identify the critical code by measurement, then optimize.

### also Typical metrics you can collect (and how to interpret them)

Common counters (exact availability depends on CPU/kernel settings):

- cycles and instructions → compute CPI = cycles / instructions

- Lower CPI often suggests better instruction-level efficiency, but you must interpret it alongside memory effects.

- cache misses (L1/LLC):

  - High last-level-cache misses often indicates memory-bound code, poor locality, or bad access patterns.

- branch misses:

    - High branch mispredict rates can point to unpredictable conditionals or data-dependent branches.