# AudioProjectTest

## Building for Windows

See [`AudioProjectTest/notes/Setup.md`](AudioProjectTest/notes/Setup.md).

## Building for Linux

### Extract `fftw-3.3.10.tar.gz` contents

```bash
tar -xvf fftw-3.3.10.tar.gz
```

Extracts the `fftw-3.3.10` folder (beside the `external/FFTW-win` folder), which contains the source files.

### Create build plan

By default, FFTW will install headers to `/usr/local/include` and libraries to `/usr/local/lib`. If you need to use a different location, specify it with the `--prefix=/not/usr/local` `configure` option and pass the paths to CMake (`cmake -DFFTW_INCLUDE_DIR=/not/usr/local/include -DFFTW_LIBRARY=/not/usr/local/lib/libfftw3f.a ..
`)

```bash
cd fftw-3.3.10
./configure --enable-static --disable-shared --enable-float
```

### Build & install FFTW

```bash
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
