[![Build Status](https://travis-ci.org/Naruto/simon-speck-c.svg?branch=develop)](https://travis-ci.org/Naruto/simon-speck-c?branch=develop)
[![Windows build status](https://ci.appveyor.com/api/projects/status/niji0dd7q1euolvk?svg=true)](https://ci.appveyor.com/project/Naruto/simon-speck)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/10443/badge.svg)](https://scan.coverity.com/projects/10443)
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bhttps%3A%2F%2Fgithub.com%2FNaruto%2Fsimon-speck-c.svg?type=shield)](https://app.fossa.io/projects/git%2Bhttps%3A%2F%2Fgithub.com%2FNaruto%2Fsimon-speck-c?ref=badge_shield)

# simon-speck-c
simon and speck are lightweight block cipher algorithms, published by NSA.([iadgov/simon-speck](https://github.com/iadgov/simon-speck))

this is one reference implementation example by C language.

support platforms are linux, iOS, Android ndk, macOS and Windows.

# Supports

- algorithms and block sizes, key sizes
    - speck
        - 128/128
        - 128/192
        - 128/256
- block cipher mode
    - ECB
    - CTR
- platforms, architectures
    - linux
        - x86_64(enable AVX2)
        - arm(enable NEON)
        - aarch64(enable NEON)
    - iOS
        - armv7(enable NEON)
        - armv7s(enable NEON)
        - arm64(enable NEON)
        - x86 (simulator)
        - x86_64 (simulator)
    - android
        - armeabi
        - armeabi-v7a(enable NEON)
        - x86
        - x86_64
        - arm64-v8a(enable NEON)
    - macOS
        - x86_64(enable AVX2)
    - windows
        - x86
        - x86_64(enable AVX2)

# Samples

```C
#include <speck/speck.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <random>

int main() {
    uint8_t key[16]; // when use 128/192, key size is 24. when use 128/256, key size is 32.
    uint8_t original_iv[16];
    uint8_t plain_text[16];
    uint8_t crypted_text[16];
    uint8_t decrypted_text[16];

    // generate key and iv
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis;

    for(int i=0; i<sizeof(key); i++) {
        key[i] = static_cast<uint8_t>(dis(gen));
    }
    for(int i=0; i<sizeof(original_iv); i++) {
        original_iv[i] = static_cast<uint8_t>(dis(gen));
    }
    snprintf(reinterpret_cast<char *>(plain_text), sizeof(plain_text), "hello world!!!!");

    speck_ctx_t *ctx = speck_init(SPECK_ENCRYPT_TYPE_128_128, key, sizeof(key));
    // ECB test
    speck_ecb_encrypt(ctx, plain_text, crypted_text, sizeof(plain_text));
    speck_ecb_decrypt(ctx, crypted_text, decrypted_text, sizeof(crypted_text));
    printf("speck 128/128 ecb\n");
    printf("  plain:     %s\n", plain_text);
    printf("  decrypted: %s\n", decrypted_text);

    // CTR test
    uint8_t iv[16];
    memcpy(iv, original_iv, sizeof(iv));
    speck_ctr_encrypt(ctx, plain_text, crypted_text, sizeof(plain_text), iv, sizeof(iv));
    memcpy(iv, original_iv, sizeof(iv));
    speck_ctr_decrypt(ctx, crypted_text, decrypted_text, sizeof(crypted_text), iv, sizeof(iv));
    printf("speck 128/128 ctr\n");
    printf("  plain:     %s\n", plain_text);
    printf("  decrypted: %s\n", decrypted_text);
}
```


# bindings

- C#
    - [Naruto/simon-speck-net](https://github.com/Naruto/simon-speck-net)

# development
## Requirements
### common

- cmake 3.7 higher

### platforms

- linux
    - gcc
- iOS & macOS
    - xcode
- android
    - Android NDK r10e higher
- windows
    - Visual Studio 2015 higher

## build
### develop build

on macOS or Linux.

```
rm -rf build
mkdir build
cd build
cmake -DENABLE_TESTING=ON -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --clean-first
ctest
```

### release build
#### linux

```
./scripts/speck/build_linux.sh
```

shared library is outputted to `libs/linux` directory.

#### iOS

```
./scripts/speck/build_ios.sh
```

fat library(simulator, device) is outputted to `libs/ios` directory.

#### android

```
./scripts/speck/build_android.sh
```

shared librares of each architectures are outputted to `libs/android`.

#### macOS

```
./scripts/speck/build_mac.sh
```

bundle file is outputted to `libs/mac` directory.

#### windows

```
scripts\speck\build_win.bat
```

dll library is outputted to `libs/windows` directory.

## benchmark

example enable avx2 benchmark on macOS or Linux.

```
rm -rf build
mkdir build
pushd build
cmake -DENABLE_TESTING=ON -DENABLE_BENCHMARK=ON -DENABLE_AVX2=ON -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --clean-first
ctest .
./test/speck/speck128128/speck128128benchmark
./test/speck/speck128192/speck128192benchmark
./test/speck/speck128256/speck128256benchmark
```

### benchmark program usage

```
./test/speck/speck128128/speck128128benchmark [test byte length] [test count]

# e.g.: test encrypt 8096 byte data by speck and speck ctr 128/128 at 50 times.
./test/speck/speck128128/speck128128benchmark 8096 50
```


# License
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bhttps%3A%2F%2Fgithub.com%2FNaruto%2Fsimon-speck-c.svg?type=large)](https://app.fossa.io/projects/git%2Bhttps%3A%2F%2Fgithub.com%2FNaruto%2Fsimon-speck-c?ref=badge_large)
