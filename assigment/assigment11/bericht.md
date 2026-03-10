## Topic: Garbage Collection (GC) & Write Amplification in SSDs

### Core idea
SSDs cannot overwrite data in place. When data becomes invalid, the SSD must copy valid pages elsewhere, erase the whole block, and then write again. This process is called **garbage collection (GC)** and it causes **write amplification** (more internal writes than requested by the application).

### Why it matters
- **Write Amplification Factor (WAF)** = physical NAND writes ÷ host writes  
  Higher WAF reduces SSD lifetime and performance.
- **TRIM support** allows the OS to inform the SSD which blocks are no longer needed, reducing GC overhead.
- **Over-provisioning** (keeping free space on the SSD) gives the controller flexibility, lowering GC cost and stabilizing performance.

### SSD-friendly design takeaway
Applications should favor **sequential and append-style writes** and avoid frequent small random overwrites to reduce garbage collection and write amplification.


## Two things I found particularly interesting

### 1) “SSD-friendly” app changes can give *another big jump*, not just “SSD is faster”
The paper shows that simply swapping HDD → SSD isn’t the end: an IO-bound app went from **142 qps (HDD)** to **20K qps (naive SSD)**, and then to **100K qps** after making the design SSD-friendly by using **more concurrent IO threads** to exploit SSD internal parallelism. 

### 2) “Don’t fill the SSD” because GC cost explodes near full capacity
A really practical point: as the SSD gets fuller, garbage collection must compact more blocks and copy far more live pages to free space. The paper explains this grows roughly with **1/(1−A)** (A = fullness), and gives a striking example: at **95% full**, freeing one block can require compacting about **20 blocks** and copying thousands of pages—hurting performance and increasing wear.