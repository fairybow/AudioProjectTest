# General Setup

## C++ Version

Configuration Properties > General > C++ Language Standard

```
ISO C++20 Standard (/std:c++20)
```

(Because we're using `std::filesystem` and `std::format`.)

## Includes

Configuration Properties > C/C++ > General > Additional Include Directories

Note: libsndfile's `sndfile.hh` includes the C header, but we need to help VS find it.

```
$(ProjectDir)external\FFTW;%(AdditionalIncludeDirectories)
```

## Linking

Configuration Properties > Linker > General > Additional Library Directories

```
$(ProjectDir)external\FFTW;%(AdditionalLibraryDirectories)
```

Configuration Properties > Linker > Input > Additional Dependencies

```
libfftw3-3.lib;$(CoreLibraryDependencies);%(AdditionalDependencies)
```

## FFTW Extra Setup

FFTW has 3 DLLs of different precision:
- libfftw3-3 (single)
- libfftw3f-3 (double)
- libfftw3l-3 (long double)

Extract entire contents of FFTW's DLL download for x64 machines to `external/FFTW`.

### Creating FFTW's .lib file for Windows

(VS terminal only to use Microsoft's `lib.exe`, which creates `.lib` files from `.def`s.)

```
cd FFTVA/external/FFTW
lib /def:libfftw3-3.def /out:libfftw3-3.lib /machine:x64
lib /def:libfftw3f-3.def /out:libfftw3f-3.lib /machine:x64
lib /def:libfftw3l-3.def /out:libfftw3l-3.lib /machine:x64
```
