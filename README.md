# simon_speck
simon and speck is lightweight block cipher algorithm, published by NSA.( https://eprint.iacr.org/2013/404.pdf )

this is one reference implementation example by C language.

support platforms are linux, iOS, Android ndk, macOS and Windows.

# Supports

- algorithms and block sizes
    - speck 128/128
- platforms
    - linux
    - iOS
    - android
    - macOS
    - windows

# Requirements
## common

- cmake 3.7 higher

## platforms

- linux
    - gcc
- iOS & macOS
    - xcode
- android
    - Android NDK r10e higher
- windows
    - Visual Studio 2015 higher

# build
## develop build

on macOS or Linux.

```
rm -rf build
mkdir build
cd build
cmake -DENABLE_TESTING=ON -DCMAKE_BUILD_TYPE=Debug ..
make
make test
```

## release build
### linux

```
./scripts/speck/build_linux.sh
```

shared library is outputted to `libs/linux` directory.

### iOS

```
./scripts/speck/build_ios.sh
```

fat library(simulator, device) is outputted to `libs/ios` directory.

### android

```
./scripts/speck/build_android.sh
```

shared librares of each architectures are outputted to `libs/android`.

### macOS

```
./scripts/speck/build_mac.sh
```

bundle file is outputted to `libs/mac` directory.

### windows

```
scripts\speck\build_win.bat
```

dll library is outputted to `libs/win` directory.

