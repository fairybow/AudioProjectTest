# Todo

- Build the library into the program instead.
- Determine implementation type (library or app)
- Potentially do not throw (just notify) and allow for reading of subsequent audio files, if provided.
- Implement wisdom in a Transformer class

## FFTW's "Wisdom" Mechanism

Since all our audio clips share the same format (raw files, 8kHz, linear 16), the Fourier transforms we need to perform are likely very similar. Creating and reusing a plan via the wisdom mechanism could significantly speed up the process.

Generating an FFTW plan can be computationally expensive. Once we've created an optimized plan for our specific transform, saving and reusing it can save time and resources in subsequent runs.

While each clip is unique, the nature of the transform we need to perform is uniform due to the consistent format. Utilizing FFTW wisdom here would enhance efficiency and performance.

Check for Saved Plan: When the program starts, it should look for a saved FFTW plan for the specific type of audio file (raw files, 8KHz, linear 16).

Load the Plan if Available: If a saved plan is found, it can be loaded and used for the transforms.

Create and Save Plan if Not Available: If no saved plan is found, the program should create a new FFTW plan and save it for future use.

This approach will optimize the performance and efficiency of your audio screening process.

## Possible next steps

It seems the current heuristic for detecting human voice may not be robust enough for your test clip. Here are some possible reasons why `containsVoice` is returning `false` and `voiceConfidence` is `0.0f`:

---

### **Potential Issues**
1. **Incorrect Frequency Band Ranges:**
   - Your `mid_freq_energy` range (bins 5–20) might not fully capture the relevant frequencies for human voice.
   - With 8 kHz sampling and `fftSize_ = 1024`, the frequency resolution is:
     \[
     \text{Frequency per bin} = \frac{\text{Sampling Rate}}{\text{FFT Size}} = \frac{8000}{1024} \approx 7.81 \, \text{Hz}
     \]
   - Bin ranges:
     - Bin 1–4: \( 7.81 \, \text{Hz} \) to \( 31.25 \, \text{Hz} \) (sub-bass, not voice-relevant)
     - Bin 5–20: \( 39.06 \, \text{Hz} \) to \( 156.25 \, \text{Hz} \) (only partially overlaps with human voice range)
   - Human voice fundamental frequencies are typically \( 85 \, \text{Hz} \)–\( 300 \, \text{Hz} \). This suggests your `mid_freq_energy` band is too narrow.

2. **Harmonic Content Ignored:**
   - Human voice has strong harmonic structure. If you're only evaluating spectral energy without considering harmonic patterns, the heuristic might fail.

3. **AI Voice Characteristics:**
   - AI-generated voices may have subtle differences in spectral energy distribution compared to natural human speech. This could confuse the heuristic.

4. **Noise or Artifacts in the Clip:**
   - Background noise or audio artifacts could shift the spectral energy distribution, reducing the confidence score.

---

### **Improvements**
1. **Refine Frequency Band Ranges:**
   - Update `mid_freq_energy` to cover a broader range, such as \( 85 \, \text{Hz} \)–\( 4 \, \text{kHz} \). In terms of bins:
     - Lower bound: \( \frac{85}{7.81} \approx 11 \)
     - Upper bound: \( \frac{4000}{7.81} \approx 512 \)
   - Adjust the `std::accumulate` calls accordingly:
     ```cpp
     float low_freq_energy = std::accumulate(total_spectral_energy.begin(), total_spectral_energy.begin() + 11, 0.0f); // <85 Hz
     float mid_freq_energy = std::accumulate(total_spectral_energy.begin() + 11, total_spectral_energy.begin() + 512, 0.0f); // 85-4000 Hz
     float high_freq_energy = std::accumulate(total_spectral_energy.begin() + 512, total_spectral_energy.end(), 0.0f); // >4000 Hz
     ```

2. **Add Harmonicity Analysis:**
   - Use autocorrelation or cepstrum analysis to identify periodicity, a key feature of human voice.
   - For now, you can look for peaks in spectral energy at integer multiples of the fundamental frequency.

3. **Adjust Confidence Calculation:**
   - Weight `mid_freq_energy` more heavily, as it's most relevant for human voice detection:
     ```cpp
     analysis.voiceConfidence = mid_freq_energy / (low_freq_energy + high_freq_energy + mid_freq_energy);
     if (analysis.voiceConfidence > 0.3f) // Adjust threshold as needed
         analysis.containsVoice = true;
     ```

4. **Visualize Spectral Energy (Debugging):**
   - Output or plot the `total_spectral_energy` to see how the energy is distributed across frequency bins. This can reveal whether the AI voice's energy is being misclassified.

---

### **Revised Code Snippet**
Here’s a refined version of the key analysis logic:
```cpp
// Calculate energy in key bands
float low_freq_energy = std::accumulate(total_spectral_energy.begin(), total_spectral_energy.begin() + 11, 0.0f); // <85 Hz
float mid_freq_energy = std::accumulate(total_spectral_energy.begin() + 11, total_spectral_energy.begin() + 512, 0.0f); // 85-4000 Hz
float high_freq_energy = std::accumulate(total_spectral_energy.begin() + 512, total_spectral_energy.end(), 0.0f); // >4000 Hz

// Refined heuristic for human voice detection
analysis.voiceConfidence = mid_freq_energy / (low_freq_energy + high_freq_energy + mid_freq_energy);
if (analysis.voiceConfidence > 0.3f) // Confidence threshold (adjust as needed)
{
    analysis.containsVoice = true;
}
else
{
    analysis.containsVoice = false;
    analysis.voiceConfidence = 0.0f;
}
```

---

### **Next Steps**
1. **Test with the updated frequency ranges and confidence calculation.**
2. **Analyze energy distribution in your test clip:** Plot or log `total_spectral_energy` to validate assumptions.
3. **Refine thresholds based on results.** Let me know how it performs, and we can adjust further!

## Voice Detection

To determine whether a given audio file contains human voice, you'll need to focus on the following key characteristics of human speech extracted from the FFT analysis:

---

### **Key Information for Voice Detection**

1. **Frequency Range of Human Voice:**
   - **Typical Range:** Human speech predominantly occupies the frequency range of **85 Hz to 300 Hz** (for fundamental frequencies) and up to **4 kHz** (for overtones and intelligibility features).
   - **What to Analyze:** Focus on the magnitude spectrum in this frequency range to detect prominent energy patterns.
   - **FFT Bins:** Map these frequency ranges to the corresponding FFT bins using the formula:
     \[
     \text{bin index} = \text{frequency} \times \frac{\text{FFT size}}{\text{sampling rate}}
     \]

2. **Formant Structure:**
   - **Definition:** Formants are resonant frequencies produced by the vocal tract. In human voice, they occur as distinct peaks in the spectrum.
   - **What to Analyze:** Look for peaks in the frequency spectrum that match the typical locations of formants:
     - F1: 250–900 Hz
     - F2: 850–2500 Hz
     - F3: 2500–3500 Hz
   - **How to Analyze:** Use peak-detection algorithms to identify and count prominent formants.

3. **Spectral Slope:**
   - **Definition:** The spectral slope describes how the energy decreases as frequency increases, which is steeper for human voice than for many other sounds.
   - **What to Analyze:** Calculate the average magnitude in low-frequency bins (e.g., below 1 kHz) and high-frequency bins (e.g., above 3 kHz). Compute the ratio or difference to estimate the slope.

4. **Voicing Detection (Harmonicity):**
   - **Definition:** Human voice often has a periodic nature due to vocal cord vibration, producing a harmonic structure.
   - **What to Analyze:** Examine whether the frequency spectrum shows a harmonic pattern (frequencies at integer multiples of a fundamental frequency).

5. **Temporal Consistency:**
   - **Definition:** Human speech shows patterns of energy that change over time in a consistent way (e.g., syllables, pauses).
   - **What to Analyze:** Track how the energy in key frequency bands (e.g., 300 Hz–3 kHz) varies over time. Speech will show smooth transitions, unlike noise or other sounds.

6. **Noise and Silence Levels:**
   - **Definition:** Human speech alternates between voiced sounds and brief silences.
   - **What to Analyze:** Detect periods of near-zero energy in the time or frequency domain. Noise and continuous sounds will lack such breaks.

7. **Energy Distribution Across Frequency Bands:**
   - **Definition:** Speech tends to have more energy concentrated in the mid-range frequencies (300 Hz–3 kHz).
   - **What to Analyze:** Compute the energy in different frequency bands and check if the distribution matches typical human voice patterns.

---

### **Practical Steps Using FFT Analysis**
1. **Divide the Magnitude Spectrum into Bands:**
   - Create frequency bands corresponding to:
     - Low (<300 Hz)
     - Mid (300 Hz–3 kHz)
     - High (>3 kHz)
   - Sum the energy within each band.

2. **Look for Peaks (Formants):**
   - Use simple peak-detection logic to identify prominent peaks in the mid-band (F1–F3 ranges).

3. **Compute Statistics Over Time:**
   - Calculate mean, variance, and temporal changes in the spectral energy within the human speech range.

4. **Harmonicity Check:**
   - Perform autocorrelation or cepstral analysis on the magnitudes to confirm harmonic structures typical of voicing.

5. **Classify Based on Thresholds:**
   - Set thresholds for energy ratios, formant counts, spectral slope, etc., based on human voice characteristics.

---

### **Why These Features?**
- The human voice exhibits distinct characteristics in terms of frequency, periodicity, and temporal dynamics that distinguish it from most other sounds.
- FFT provides a time-frequency representation, making it suitable for extracting these features.

---

### **Next Steps**
You can start by hardcoding thresholds for key features (e.g., frequency range, energy distribution, harmonicity) and refining them based on sample data. Later, these thresholds can be adjusted dynamically or through statistical analysis for better accuracy. Would you like to explore any of these features in greater detail or start coding specific parts of this logic?
