# Setup

## VS Configuration Properties

### General

C++ Language Standard

```
ISO C++17 Standard (/std:c++17)
```

### Debugging

Add test command args (flags, files) here.

### C/C++

General > Additional Include Directories

```
$(ProjectDir)external\FFTW-win;%(AdditionalIncludeDirectories)
```

### Linker

General > Additional Library Directories

```
$(ProjectDir)external\FFTW-win;%(AdditionalLibraryDirectories)
```

Input > Additional Dependencies

```
libfftw3f-3.lib;$(CoreLibraryDependencies);%(AdditionalDependencies)
```

### Build Events

Pre-Build Event > Command Line

```
call "$(ProjectDir)scripts\VSWinPreBuild.bat" "$(ProjectDir)external\" "$(TargetDir)"
```

## FFTW Extra Setup

FFTW has 3 DLLs of different precision (in order):
- libfftw3f-3 (float)
- libfftw3-3 (double) (default)
- libfftw3l-3 (long double)

Extract entire contents of FFTW's DLL download for x64 machines to `external/FFTW-win`.

### Creating FFTW's .lib file for Windows

(VS terminal only to use Microsoft's `lib.exe`, which creates `.lib` files from `.def`s.)

```
cd AudioProjectTest/external/FFTW-win
lib /def:libfftw3-3.def /out:libfftw3-3.lib /machine:x64
lib /def:libfftw3f-3.def /out:libfftw3f-3.lib /machine:x64
lib /def:libfftw3l-3.def /out:libfftw3l-3.lib /machine:x64
```
