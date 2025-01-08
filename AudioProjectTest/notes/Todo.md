# Todo

Your code appears to be working as intended, especially since you're seeing improved detection with more timestamps being recorded due to the overlap. However, the **3x increase in computation time** suggests there might be room for optimization. Below, I provide feedback and potential areas for improvement.

---

### Observations

1. **Improved Detection**:
   - The increase in timestamps aligns with expectations, as overlapping windows provide more analysis points, improving sensitivity.

2. **Increased Computation Time**:
   - The 3x increase in runtime is likely due to overlapping chunks, effectively increasing the number of FFT computations. For 50% overlap, the number of windows approximately doubles.

3. **Code Logic**:
   - Your logic for handling overlap (e.g., `hop_size`, `chunks_count`, and remainder handling) is correct and working as expected.

---

### Optimizations to Consider

#### 1. **Minimize Redundant I/O**
The `raw_audio.seekg()` call within the loop adds significant overhead, as it involves adjusting the file stream position repeatedly. Instead of performing a `seekg` after every chunk, you could:

- **Maintain a Sliding Buffer**:
   Use a buffer large enough to hold multiple overlapping windows and only read from the file once per block of data. This avoids frequent seek operations.

   Example:
   ```cpp
   std::vector<std::int16_t> sliding_buffer(m_fftSize + hop_size);
   raw_audio.read(reinterpret_cast<char*>(sliding_buffer.data()), sliding_buffer.size() * sizeof(std::int16_t));

   for (std::size_t chunk_i = 0; chunk_i < chunks_count; ++chunk_i) {
       auto chunk_start_time = static_cast<float>(chunk_i * hop_size) / SAMPLING_RATE;
       std::vector<std::int16_t> chunk(sliding_buffer.begin() + chunk_i * hop_size,
                                       sliding_buffer.begin() + chunk_i * hop_size + m_fftSize);

       _fftAnalyzeChunk(chunk, chunk_start_time, static_chunk_start_times);
   }
   ```

   This approach eliminates the need for `seekg`, reducing I/O overhead.

---

#### 2. **Leverage AVX2 Optimizations**
You already have AVX2 optimizations in place, but additional steps can improve performance:
- **Batch Magnitude Computation**:
   Use SIMD to compute magnitudes in batches, reducing loop overhead.
- **Windowing**:
   Precompute Hann window coefficients and align them for AVX2 operations.

---

#### 3. **Parallelize FFT Computation**
You can parallelize chunk processing by dividing chunks among multiple threads using a thread pool or parallel execution library (e.g., OpenMP or std::async):

```cpp
std::vector<std::future<void>> futures;
for (std::size_t chunk_i = 0; chunk_i < chunks_count; ++chunk_i) {
    futures.push_back(std::async(std::launch::async, [&]() {
        auto chunk_start_time = static_cast<float>(chunk_i * hop_size) / SAMPLING_RATE;
        std::vector<std::int16_t> chunk(buffer.begin() + chunk_i * hop_size,
                                        buffer.begin() + chunk_i * hop_size + m_fftSize);
        _fftAnalyzeChunk(chunk, chunk_start_time, static_chunk_start_times);
    }));
}

for (auto& f : futures) {
    f.get();
}
```

---

#### 4. **Reduce Remainder Overhead**
Currently, the remainder is handled separately, which adds a small amount of additional overhead. You could integrate the remainder into the main loop by zero-padding it on the fly.

---

### Benchmarking the Improvements
To assess the effectiveness of these optimizations:
1. Measure runtime before and after each change.
2. Ensure results (timestamps and detection) remain consistent.

---

### Summary
The code looks solid and performs as expected for overlap-based analysis. To reduce the computation time:
1. Eliminate redundant I/O using a sliding buffer.
2. Further leverage AVX2 or parallel processing for FFT computations.
3. Integrate remainder handling into the main loop.
