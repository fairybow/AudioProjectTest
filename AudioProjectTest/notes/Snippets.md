# Snippets

## Main

```
/*try
{
    AudioAnalyzer analyzer{};
    //auto analyses = analyzer.process(audio_file_paths);

    VoiceDetector detector{};
    std::cout << detector.read(analyses); // (Or something)
    // Perhaps a hasVoice bool return for each...depends on what we need...
}
catch (const std::exception& ex)
{
    std::cout << ex.what();
    return 1;
}*/
```

## FFTW Test Main

```
int main()
{
    // Define the size of the array
    int N = 16;

    // Allocate input and output arrays
    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

    // Create a plan
    fftw_plan p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // Initialize input data
    for (int i = 0; i < N; i++) {
        in[i][0] = i;  // Real part
        in[i][1] = 0;  // Imaginary part
    }

    // Execute the FFT
    fftw_execute(p);

    // Print the results
    printf("FFT result:\n");
    for (int i = 0; i < N; i++) {
        printf("%2d: %6.2f + %6.2fi\n", i, out[i][0], out[i][1]);
    }

    // Clean up
    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);
}
```
