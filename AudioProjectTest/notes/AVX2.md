# AVX2

To implement the functionality you described, we need to process raw audio files (sampled at 8 kHz, 16-bit linear encoding) and perform an FFT analysis on them. When `USE_AVX2` is defined, the goal is to leverage SIMD instructions to process multiple files in parallel. Here's how you can implement this step-by-step:

---

### **1. Structure of the Function**

We'll design the `AudioAnalyzer::process` function to handle two paths:
- **With AVX2 (`USE_AVX2`)**: Load data from multiple files into SIMD-friendly buffers and process them in parallel.
- **Without AVX2**: Process files sequentially, one at a time.

### **2. Memory Alignment and FFTW**
FFTW requires SIMD-aligned memory for maximum performance when using AVX2. This means:
- Allocate memory using `fftw_malloc`.
- Free memory using `fftw_free`.

---

### **3. Steps for the Function**

Here’s the plan:

1. **Input Validation**:
   Ensure all file paths are valid and accessible.

2. **Read Audio Data**:
   Read raw audio data (16-bit linear PCM) from the input files into memory.

3. **Preprocess for SIMD (if `USE_AVX2` is defined)**:
   Use aligned buffers for SIMD operations. Group and interleave data for parallel processing.

4. **Perform FFT Analysis**:
   Call the FFT analysis function, whether in parallel (AVX2) or sequentially.

5. **Return Results**:
   Return a vector of `AudioAnalyzer::Analysis` objects containing analysis results.

---

### **Example Implementation**

```cpp
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <fftw3.h>
#include <immintrin.h> // For AVX2

namespace AudioAnalyzer {
    struct Analysis {
        std::vector<double> fftResult; // Example FFT result
        std::filesystem::path filePath; // Associated file
    };

    std::vector<Analysis> process(const std::vector<std::filesystem::path>& inFiles) {
        if (inFiles.empty()) {
            throw std::invalid_argument("No input files provided.");
        }

        std::vector<Analysis> results;

#if defined(USE_AVX2)
        // Load all files into memory and align data for SIMD processing
        std::size_t totalSamples = 0;
        std::vector<std::int16_t*> alignedData; // Store aligned pointers
        std::vector<std::size_t> sampleCounts; // Sample count per file

        for (const auto& file : inFiles) {
            // Open and read raw audio file
            std::ifstream inputFile(file, std::ios::binary);
            if (!inputFile) {
                throw std::runtime_error("Failed to open file: " + file.string());
            }

            // Determine file size and read data
            inputFile.seekg(0, std::ios::end);
            std::size_t fileSize = inputFile.tellg();
            inputFile.seekg(0, std::ios::beg);

            if (fileSize % sizeof(std::int16_t) != 0) {
                throw std::runtime_error("File size is not a multiple of 16-bit samples.");
            }

            std::size_t sampleCount = fileSize / sizeof(std::int16_t);
            sampleCounts.push_back(sampleCount);
            totalSamples += sampleCount;

            // Allocate aligned memory for SIMD
            std::int16_t* alignedBuffer = reinterpret_cast<std::int16_t*>(fftw_malloc(sampleCount * sizeof(std::int16_t)));
            if (!alignedBuffer) {
                throw std::runtime_error("Failed to allocate aligned memory.");
            }

            // Read data into aligned buffer
            inputFile.read(reinterpret_cast<char*>(alignedBuffer), fileSize);
            alignedData.push_back(alignedBuffer);
        }

        // Perform SIMD processing (FFT in parallel)
        // Example: Interleave or process all data
        std::cout << "Processing files in parallel with AVX2..." << std::endl;

        // Simulate FFT analysis (implement your actual FFT here)
        for (std::size_t i = 0; i < inFiles.size(); ++i) {
            // Dummy FFT result
            Analysis analysis;
            analysis.filePath = inFiles[i];
            analysis.fftResult = { 0.0 }; // Replace with actual FFT result
            results.push_back(std::move(analysis));
        }

        // Free aligned memory
        for (auto* buffer : alignedData) {
            fftw_free(buffer);
        }

#else // !defined(USE_AVX2)

        // Process files sequentially
        for (const auto& file : inFiles) {
            std::cout << "Processing file: " << file << std::endl;

            // Open and read raw audio file
            std::ifstream inputFile(file, std::ios::binary);
            if (!inputFile) {
                throw std::runtime_error("Failed to open file: " + file.string());
            }

            // Determine file size and read data
            inputFile.seekg(0, std::ios::end);
            std::size_t fileSize = inputFile.tellg();
            inputFile.seekg(0, std::ios::beg);

            if (fileSize % sizeof(std::int16_t) != 0) {
                throw std::runtime_error("File size is not a multiple of 16-bit samples.");
            }

            std::size_t sampleCount = fileSize / sizeof(std::int16_t);
            std::vector<std::int16_t> audioData(sampleCount);
            inputFile.read(reinterpret_cast<char*>(audioData.data()), fileSize);

            // Perform FFT analysis
            // Example: Simulate FFT result
            Analysis analysis;
            analysis.filePath = file;
            analysis.fftResult = { 0.0 }; // Replace with actual FFT result
            results.push_back(std::move(analysis));
        }

#endif // defined(USE_AVX2)

        return results;
    }
}
```

---

### **Key Points**

1. **SIMD Path (`USE_AVX2`):**
   - Use `fftw_malloc` and `fftw_free` to ensure alignment.
   - Interleave or batch-process multiple files' data in parallel.
   - Perform FFT on the aligned buffers.

2. **Non-SIMD Path:**
   - Process files sequentially.
   - Allocate memory using standard containers like `std::vector`.

3. **FFT Implementation:**
   Replace the dummy FFT logic with a proper FFT implementation using libraries like FFTW or your custom code.

4. **Error Handling:**
   Include robust error handling for file reading, alignment, and memory allocation.

This structure provides a flexible and efficient way to handle both SIMD and non-SIMD paths for your audio processing application.

---

```
//#define USE_AVX2

#if defined(USE_AVX2)
#include <immintrin.h>
#pragma message("Using AVX2 SIMD.")

// Function to perform element-wise addition of two arrays of 16-bit integers
inline void vectorAdd
(
    std::int16_t* arrayA,
    std::int16_t* arrayB,
    const std::size_t arrayLength
)
{
    std::size_t index = 0;

    // Process 16 elements at a time using AVX2 SIMD instructions
    for (; index + 15 < arrayLength; index += 16)
    {
        std::cout << "Processing SIMD index range: " << index << " to " << index + 15 << std::endl;

        __m256i vector_a = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(arrayA + index));
        __m256i vector_b = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(arrayB + index));
        __m256i result_vector = _mm256_add_epi16(vector_a, vector_b);
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(arrayA + index), result_vector);
    }

    // Handle remaining elements using scalar addition
    for (; index < arrayLength; ++index)
    {
        std::cout << "Processing scalar index: " << index << std::endl;
        arrayA[index] += arrayB[index];
    }
}

#endif
```
