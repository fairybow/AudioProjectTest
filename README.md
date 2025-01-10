# AudioProjectTest

## Command Line Flags

### `--fft-size=x`

The size of analyzed sample chunks. FFTW accepts nearly any value but works best with multiples of 2 (common sizes are [1024, 2048, and 4096](https://dobrian.github.io/cmp/topics/fourier-transform/1.getting-to-the-frequency-domain-theory.html)).

Valid values: Any positive value.

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

The read/write path for FFTW wisdom.

Valid values: Any-ish.

Default value: None (do not use FFTW wisdom).

## Building for Windows

See [`AudioProjectTest/notes/Setup.md`](AudioProjectTest/notes/Setup.md).

## Building for Linux

### Extract FFTW `.tar.gz` contents

```bash
tar -xvf fftw-3.3.10.tar.gz
```

Extracts the `fftw-3.3.10` folder (beside the `external/FFTW-win` folder), which contains the source files.

### Build & install FFTW

By default, FFTW will install headers to `/usr/local/include` and libraries to `/usr/local/lib`. If you need to use a different location, specify it with the `--prefix=/not/usr/local` `configure` option and pass the paths to CMake (`cmake -DFFTW_INCLUDE_DIR=/not/usr/local/include -DFFTW_LIBRARY=/not/usr/local/lib/libfftw3f.a ..
`)

```bash
cd fftw-3.3.10
./configure --enable-static --disable-shared --enable-float
make
sudo make install
```

### Build project

The project directory should be just one up from `fftw-3.3.10` after extraction.

```bash
cd ../
mkdir build
cd build
cmake ..
make
```
