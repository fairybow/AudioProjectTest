# AVX2

```
//#define USE_AVX2

#if defined(USE_AVX2)
#include <immintrin.h>
#pragma message("Using AVX2 SIMD.")

// Function to perform element-wise addition of two arrays of 16-bit integers
void vectorAdd(int16_t* arrayA, int16_t* arrayB, const size_t arrayLength)
{
    size_t index = 0;

    // Process 16 elements at a time using AVX2 SIMD instructions
    for (; index + 15 < arrayLength; index += 16)
    {
        // Load 16 elements from each array into AVX2 registers
        __m256i vector_a = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(arrayA + index));
        __m256i vector_b = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(arrayB + index));

        // Perform element-wise addition
        __m256i result_vector = _mm256_add_epi16(vector_a, vector_b);

        // Store the result back into arrayA
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(arrayA + index), result_vector);
    }

    // Handle remaining elements using scalar addition
    for (; index < arrayLength; ++index)
    {
        arrayA[index] += arrayB[index];
    }
}

#endif
```
