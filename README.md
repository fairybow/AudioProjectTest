# AudioProjectTest

## Building for Windows

See [`AudioProjectTest/notes/Setup.md`](AudioProjectTest/notes/Setup.md).

## Building for Linux

### 1. Prereqs

```bash
sudo apt update
sudo apt-get install git
sudo apt install cmake
sudo apt install build-essential
```

### 2. Setup

```bash
git clone https://github.com/fairybow/AudioProjectTest
chmod +x AudioProjectTest/AudioProjectTest/scripts/LinuxBuild.sh
```

### 3. Build

```bash
AudioProjectTest/AudioProjectTest/scripts/LinuxBuild.sh -DUSE_AVX2=ON -DUSE_DX_BENCH_MACROS=ON -DUSE_LOGGING=ON
```

Find the executable at `AudioProjectTest/AudioProjectTest/build`.

Run like:

```bash
cd AudioProjectTest/AudioProjectTest/build
./AudioProjectTest { file }.raw --wisdom=./wisfile
```

## Command Line Flags

### `--fft-size=x`

The size of analyzed sample chunks. FFTW accepts nearly any value but works best with multiples of 2 (common sizes are [1024, 2048, and 4096](https://dobrian.github.io/cmp/topics/fourier-transform/1.getting-to-the-frequency-domain-theory.html)).

Valid values: Any positive integer.

Default value: 1024.

### `--window=x`

The desired windowing function.

Valid values: None, Triangular, Hann, Hamming, Blackman, FlatTop, or Gaussian.

Default value: Hann.

### `--overlap=x`

The sample chunk overlap percentage.

Valid values: Any value from 0.0 to 0.9.

Default value: 0.5.

### `--wisdom=x`

The read/write path for FFTW [wisdom](https://fftw.org/fftw3_doc/Words-of-Wisdom_002dSaving-Plans.html).

Valid values: Writeable (parent directory exists), system-appropriate path (`--wisdom=./wisdom` or `--wisdom=C:/Dev/fftwf_wisdom.dat`).

Default value: None.
