# Notes

Our audio files are headerless (raw), 8 kHz sampling, linear 16 encoding (meaning 2 byte samples in linear PCM encoding).

## FFT

The signal is divided into overlapping chunks (frames), each the size of the FFT. Each FFT computes the frequency domain representation for one chunk of the audio signal.

## Windows

A window is a mathematical function applied to each frame of audio samples before performing the FFT. Its purpose is to reduce spectral leakage, which occurs when discontinuities at the boundaries of the frame introduce artifacts into the frequency spectrum.

A window function is a curve that scales the amplitude of audio samples in a frame.
The function typically emphasizes the center of the frame while tapering the edges to zero.
Applying a window minimizes the abrupt transitions at frame boundaries, improving the accuracy of the FFT.

## FFT Wisdom

FFTW generates customized instructions to optimize each FFT computation based on its specific parameters. The wisdom file stores precomputed optimization data, enabling the library to reuse efficient plans and avoid the time-consuming process of recalculating them for every program run.

Wisdom files are cumulative, meaning new plans (e.g., for a different FFT size or configuration) can be added to the file and reused in subsequent runs. This makes FFTW increasingly efficient as it encounters and optimizes for new use cases.

## FFT Sizes

The FFT size determines the frequency resolution and impacts computation time. Using a constant FFT size simplifies the code and ensures consistent results. Larger FFT sizes provide better frequency resolution but take longer to compute. Smaller sizes are generally faster, but if they are too small, more computations may be needed overall, which can also increase processing time. The key is to choose a size that balances resolution and efficiency.

### Smaller FFT Sizes Are Faster

- Smaller FFT sizes require fewer computations:
    - An FFT of size `N` typically requires `O(N log N)` operations.
    - Reducing `N` significantly reduces computation time.
- However, smaller FFT sizes also have:
    - Lower frequency resolution (wider bins in the frequency domain).
    - Less data processed per transform, which might necessitate more transformations for large datasets.

### FFT Sizes as Constants

In many applications, FFT sizes are chosen as a constant (e.g., 512, 1024, 2048) because:

- It simplifies implementation.
- It aligns with application-specific needs (e.g., audio processing often uses powers of 2 for performance).
- FFTW can precompute an optimized plan for that size, which improves runtime efficiency.

Many applications (e.g., audio spectrum analysis, speech processing) use a fixed chunk size for input signals, which directly corresponds to a fixed FFT size. For example, in audio processing, we might split audio into overlapping frames of a fixed length (e.g., 1024 samples) for analysis.

### FFTW Is Flexible

FFTW can handle any size we give it, even sizes that are not powers of 2. However, FFTW is typically faster with sizes that are powers of 2 and have small prime factors (e.g., `2^k * 3^m * 5^n`). Using an odd or arbitrary size might result in less efficient computations.

### How to Choose an FFT Size

The FFT size determines the frequency resolution of the analysis and should be chosen based on the sampling rate (8 kHz in our case):

The frequency resolution is calculated as:

`Frequency Resolution = Sampling Rate / FFT Size`

For example:
- An FFT size of 1024 at 8 kHz gives frequency bins of `8000 / 1024 ~= 7.8 Hz`.
- An FFT size of 512 gives bins of `8000 / 512 = 15.6 Hz`.

Typical FFT sizes for audio processing are 256, 512, 1024, or 2048 samples. **1024 is a common choice**, as it provides a good balance between frequency resolution and computational efficiency for many applications.

If the need arises to support multiple FFT sizes in the future, the design can be revisited. For now, using a constant FFT size simplifies the implementation and ensures consistent results.
