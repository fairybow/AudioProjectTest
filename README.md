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

### 2. Clone repository

Clone, then navigate to `external`.

```bash
git clone https://github.com/fairybow/AudioProjectTest`
cd AudioProjectTest/AudioProjectTest/external
```

### Extract FFTW source code

The FFTW archive is in the external folder (beside the Windows DLL archives). It can also be downloaded [here](https://www.fftw.org/download.html).

Extract it and then navigate inside.

```bash
tar -xvf fftw-3.3.10.tar.gz
cd fftw-3.3.10
```

### Build & install FFTW

Configure FFTW and build & install it to the system. Then navigate up (to the [project directory](AudioProjectTest)).

```bash
./configure --enable-static --disable-shared --enable-float
make
sudo make install
cd ../
```

> [!NOTE]
> By default, FFTW will install headers to `/usr/local/include` and libraries to `/usr/local/lib`. If you need to use a different location, specify it with the `--prefix=/not/usr/local` `configure` option and pass the paths to CMake (`cmake -DFFTW_INCLUDE_DIR=/not/usr/local/include -DFFTW_LIBRARY=/not/usr/local/lib/libfftw3f.a ../..
`)

### Build project

Make a build directory, enter it, then build the project with CMake.

```bash
mkdir build
cd build
cmake -USE_AVX2=ON -DUSE_DX_BENCH_MACROS=ON -DUSE_LOGGING=ON ../..
make
```

> [!NOTE]
> `USE_LOGGING` may become a run-time `--verbose` flag later.

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

Valid values: Writeable path (`--wisdom=./wisdom` or `--wisdom=C:/Dev/fftwf_wisdom.dat`).

Default value: None.
