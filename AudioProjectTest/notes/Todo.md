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
