# Selected slide from the lecture and report

## Slide I selected
I selected slide 3 in the lecture PDF.
It is the slide called Example Fused Multiply Add FMA.
It shows the intrinsic `_mm256_fmadd_ps` and a small table with latency and throughput for several CPU architectures.
It also gives short definitions of latency and throughput. 

## What the intrinsic does on that slide
`_mm256_fmadd_ps` multiplies eight float values in a and b and then adds c.
It does this in one vector result.
The slide also notes that the intrinsic can map to different FMA instruction forms. 


## What latency tells you
Latency tells you how long you must wait until the result of an intrinsic is ready to be used by the next operation that depends on it.

A simple way to think about it is a chain of operations where each step needs the previous result. In that case latency limits the speed, because you cannot continue until the result exists.

This is also how latency is commonly measured, by running a long chain of the same instruction where each one depends on the previous one. 

In the lecture slide, the latency for `_mm256_fmadd_ps` is shown as 4 cycles on Skylake. That means if you do a dependent chain of fused multiply add operations, each step can only move forward after about 4 cycles. 

## What throughput tells you
Throughput tells you how often the CPU can start a new operation of the same kind when there are no data dependencies forcing it to wait.

This is about many independent operations, not a dependency chain. It is often called reciprocal throughput and it is commonly shown as cycles per instruction.

Intel describes throughput as the number of clock cycles required to wait before the CPU ports are free to accept the same instruction again. 

Agner Fog also explains throughput as something you get from a long sequence of independent instructions. 

In the lecture slide, the throughput for `_mm256_fmadd_ps` on Skylake is 0.5 cycles per instruction. This means the CPU can start about two independent fused multiply add operations per cycle. 

## Why both metrics matter for intrinsic performance
Latency matters when your code has dependencies.

Example
One accumulator
You do sum equals fmadd of sum and something
Each step depends on the previous sum
Here the latency becomes the main limit

Throughput matters when your code has enough independent work.

Example
Multiple accumulators
You compute several partial sums in parallel
Now the CPU can overlap work and the limit becomes throughput

A common optimization idea is to unroll the loop and use multiple independent accumulators so the CPU can keep starting new operations while older ones are still finishing. This helps you move from being latency limited to being throughput limited.

# Two things I found particularly interesting in
# Analyzing Vectorized Hash Tables Across CPU Architectures

## 1 SIMD speedups require changing the hash table design not only adding vector instructions

The paper compares three vectorized designs that build on each other.

Vectorized linear probing VLP  
Vectorized fingerprinting VFP  
Bucket based comparison BBC 

What I found interesting is that the most direct idea, which is to vectorize linear probing, often does not give strong speedups.
VLP adds overhead such as extra loads, handling validity, alignment issues, and extracting match positions.
When probe sequences are short, the scalar version can still be competitive or faster. 

The paper then shows why VFP and especially BBC work better.
VFP compares small fingerprints so it can check many entries at once.
It can also avoid a separate validity array by using a special fingerprint value for empty slots.
BBC groups entries into buckets so fingerprints and key value pairs are close in memory, and loads are more aligned and cache friendly.
BBC also uses an overflow flag to stop earlier in many cases.

In the evaluation, BBC can clearly beat strong scalar baselines, especially at high load factors.
Figures in the paper show these gains across Intel, AMD, ARM, and Power systems. 

## 2 Fingerprint choice and SIMD details strongly affect performance and the best option depends on the CPU

A detail I liked is the analysis of fingerprint bit selection.
The paper shows that choosing better fingerprint bits can reduce fingerprint collisions a lot.
They give an example where collisions per find drop from 0.53 to 0.03 when switching to a better bit choice.
That directly increases lookup throughput because there is less collision handling work. 

They also discuss the tradeoff between 8 bit and 16 bit fingerprints.
16 bit fingerprints reduce collisions, but fewer fingerprints fit into a SIMD register, which can increase the number of probes.
In many high load factor settings, 8 bit fingerprints can still win because vector probes are cheaper and the overall control flow is simpler.

The paper also makes a strong cross architecture point.
The same vectorized idea can behave differently on x86, ARM, and Power because the SIMD instruction sets and useful primitives differ.
So you cannot assume that an approach tuned for one architecture will automatically be best on another. 
