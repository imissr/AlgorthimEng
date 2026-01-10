
## 1) Name some characteristics of the instruction sets: SSE, AVX(2) and AVX-512

- **SSE (SSE–SSE4.2)**
    - **128-bit** SIMD vectors
    - Uses **XMM registers** (`xmm0–xmm15`)
    - Introduced roughly **1999–2009**

- **AVX / AVX2**
    - **256-bit** SIMD vectors
    - Uses **YMM registers** (`ymm0–ymm15`)
    - Launched **2011 (AVX)** and **2013 (AVX2)**
    - Common on many modern mainstream CPUs (especially AVX2)

- **AVX-512**
    - **512-bit** SIMD vectors
    - Uses **ZMM registers** (`zmm0–zmm31`) → **more registers** than SSE/AVX2
    - Launched around **2017**
    - ZMM registers conceptually contain the lower YMM/XMM parts

## 2) How can memory aliasing affect performance?

If the compiler must assume **two pointers might refer to the same memory location** (“aliasing”), it has to be conservative.  
That can prevent optimizations such as:
- reordering loads/stores,
- keeping values in registers longer,
- and especially **auto-vectorizing** loops.

If you can guarantee no overlap (e.g., using `restrict` / `__restrict__`), the compiler can optimize more aggressively.

## 3) Advantages of unit stride (stride-1) memory access vs larger strides (e.g., stride-8)

- **Better cache efficiency / higher effective bandwidth**  
  Stride-1 touches memory sequentially, so cache lines are well utilized and hardware prefetching works effectively.

- **Easier vectorization**  
  Contiguous elements map naturally to SIMD loads/stores.  
  Larger strides often lead to inefficient access (wasted cache-line data) and may require scatter/gather-like patterns.

## 4) When would you prefer arranging records in memory as a Structure of Arrays (SoA)?

Prefer **SoA** when you often process **one field across many records** (e.g., sum/average/min/max of `x` for many objects).  
SoA stores each field contiguously, improving:
- spatial locality,
- memory bandwidth usage,
- and auto-vectorization opportunities.

**Trade-off:** accessing many fields of a single record may become less local because the record’s fields are stored in separate arrays.
