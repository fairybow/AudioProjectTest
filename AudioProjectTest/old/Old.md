## Plan (OLD, was using libsndfile)

1. **Setup and Initialization**:

    - Ensure you have `libsndfile` and `FFTW` installed and properly linked in your project. Include the appropriate headers in your source file.

2. **Reading the Audio File**:

    - Use `libsndfile` to open and read the audio file. This involves:
        1. Opening the file using `SndfileHandle`.
        2. Checking the file properties such as sample rate, number of channels, and frames.
        3. Reading the audio data into a buffer (an array or a vector).

3. **Preparing the Audio Data**:

    - Ensure the audio data is in a format suitable for FFTW. If your audio has multiple channels, you might want to work with a single channel (e.g., mono) or preprocess it to combine channels.

4. **Performing the Discrete Fourier Transform (DFT)**:

    - Initialize FFTW plans for performing the DFT. This involves creating input and output arrays and planning the transformation.
    - Execute the FFT using FFTW to get the frequency domain representation of the audio data.

5. **Analyzing the Frequency Domain Data**:

    - Inspect the frequency domain data to identify characteristics of human voice. Human voice typically falls within the frequency range of approximately 85 Hz to 255 Hz for male voices and 165 Hz to 255 Hz for female voices.
    - Check for the presence of energy peaks in this frequency range. The presence of these peaks can be an indicator of human voice.

6. **Decision Making**:

    - Based on the analysis, determine if the audio contains human voice by evaluating the energy levels in the expected frequency range.

### Example Implementation Steps

1. **Include Libraries**:

```cpp
#include <iostream>
#include <sndfile.hh>
#include <fftw3.h>
#include <vector>
```

2. **Read Audio File**:

```cpp
const std::string filename = "path/to/audio.wav";
SndfileHandle infile(filename);

if (!infile) {
    std::cerr << "Error: could not open file." << std::endl;
    return 1;
}

int num_samples = infile.frames() * infile.channels();
std::vector<double> audio_data(num_samples);
infile.read(&audio_data[0], num_samples);
```

3. **Prepare Data for FFTW**:

```cpp
fftw_complex* fft_output = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * num_samples);
fftw_plan plan = fftw_plan_dft_r2c_1d(num_samples, &audio_data[0], fft_output, FFTW_ESTIMATE);
```

4. **Execute FFT**:

```cpp
fftw_execute(plan);
```

5. **Analyze Frequency Data**:

```cpp
for (int i = 0; i < num_samples / 2; ++i) {
    double frequency = i * infile.samplerate() / num_samples;
    double magnitude = sqrt(fft_output[i][0] * fft_output[i][0] + fft_output[i][1] * fft_output[i][1]);

    if (frequency >= 85 && frequency <= 255) {
        // Check for peaks in this range
        if (magnitude > threshold) {
            std::cout << "Human voice detected at frequency: " << frequency << " Hz" << std::endl;
        }
    }
}
```

6. **Clean Up**:

```cpp
fftw_destroy_plan(plan);
fftw_free(fft_output);
```
