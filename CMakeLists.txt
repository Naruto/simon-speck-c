# ----------------------------------------------------------------------------
# Root CMake file for simon and speck
# ----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.7)

project(simon_speck CXX C)


# ----------------------------------------------------------------------------
# simon speck options
# ----------------------------------------------------------------------------

# simon speck build components
# ===================================================
option(ENABLE_STATIC "Build a static version of the libraries?" OFF)
option(DISABLE_SHARED "Do not build a shared version of the libraries?" OFF)
option(ENABLE_TESTING "Enable ctest?" OFF)


# simon speck build options
# ===================================================
option(ENABLE_NEON "Enable NEON instructions" OFF)
option(ENABLE_AVX2 "Enable AXV2 instructions" OFF)


# ----------------------------------------------------------------------------
# detect compiler and target platform architecture
# ----------------------------------------------------------------------------
# include(cmake/OpenCVDetectCXXCompiler.cmake)

# Add these standard paths to the search paths for FIND_LIBRARY
# to find libraries from these locations first
if(UNIX AND NOT ANDROID)
  if(X86_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8)
    if(EXISTS /lib64)
      list(APPEND CMAKE_LIBRARY_PATH /lib64)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /lib)
    endif()
    if(EXISTS /usr/lib64)
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib64)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib)
    endif()
  elseif(X86 OR CMAKE_SIZEOF_VOID_P EQUAL 4)
    if(EXISTS /lib32)
      list(APPEND CMAKE_LIBRARY_PATH /lib32)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /lib)
    endif()
    if(EXISTS /usr/lib32)
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib32)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib)
    endif()
  endif()
endif()

# Add these standard paths to the search paths for FIND_PATH
# to find include files from these locations first
if(MINGW)
  if(EXISTS /mingw)
      list(APPEND CMAKE_INCLUDE_PATH /mingw)
  endif()
  if(EXISTS /mingw32)
      list(APPEND CMAKE_INCLUDE_PATH /mingw32)
  endif()
  if(EXISTS /mingw64)
      list(APPEND CMAKE_INCLUDE_PATH /mingw64)
  endif()
endif()


# ----------------------------------------------------------------------------
#  Detect endianness of build platform
# ----------------------------------------------------------------------------

#if(IOS)
#  # test_big_endian needs try_compile, which doesn't work for iOS
#  # http://public.kitware.com/Bug/view.php?id=12288
#  set(WORDS_BIGENDIAN 0)
#else()
#  include(TestBigEndian)
#  test_big_endian(WORDS_BIGENDIAN)
#endif()


#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

# architectures
# FIXME
if((IOS_ARCH MATCHES "armv7") OR (CMAKE_ANDROID_ARCH_ABI MATCHES armeabi-v7a) OR (CMAKE_C_COMPILER MATCHES ".*arm-linux-gnueabi-gcc$"))
  set(ARM ON)
elseif((IOS_ARCH MATCHES "arm64") OR (CMAKE_ANDROID_ARCH_ABI MATCHES arm64-v8a) OR (CMAKE_C_COMPILER MATCHES ".*aarch64-linux-gnu-gcc$"))
  set(AARCH64 ON)
endif()

# ARM NEON
if(ENABLE_NEON)
  if(ARM OR AARCH64)
    if(AARCH64)
      set(neon_flags "-O3")
    elseif(ARM)
      set(neon_flags "-mfloat-abi=softfp -mfpu=neon")
    endif()

    try_compile(NEON_FOUND "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/cmake/checks/cpu_neon.c"
      COMPILE_DEFINITIONS "${neon_flags}")
    if(NEON_FOUND)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${neon_flags}")
      set(WITH_NEON ON)
    endif()
  endif()
endif()

# Intel AVX2
if(ENABLE_AVX2)
  if(MSVC)
    set(avx2_flags "/arch:AVX2")
  else()
    set(avx2_flags "-mavx2")
  endif()
  try_compile(AVX_FOUND "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_SOURCE_DIR}/cmake/checks/cpu_avx2.c"
    COMPILE_DEFINITIONS "${avx2_flags}")

  if(AVX_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${avx2_flags}")
    set(WITH_AVX2 ON)
  endif()
endif()


# source
set(libspeck_SRCS ${libspeck_SRCS} src/speck/speck_common.c)
if(WITH_NEON)
  set(libspeck_SRCS ${libspeck_SRCS} src/speck/speck_neon.c)
elseif(WITH_AVX2)
  set(libspeck_SRCS ${libspeck_SRCS} src/speck/speck_avx2.c)
else()
  set(libspeck_SRCS ${libspeck_SRCS} src/speck/speck.c)
endif()
if(WIN32)
  set(libspeck_SRCS ${libspeck_SRCS} src/speck/dllmain.c)
endif(WIN32)

# header
set(libspeck_PUBLIC_HEADER include/speck/speck.h)
set(libspeck_PRIVATE_HEADER src/speck/speck_private.h)
include_directories(${CMAKE_SOURCE_DIR}/include)
set(libspeck_SRCS ${libspeck_SRCS} ${libspeck_PUBLIC_HEADER} ${libspeck_PRIVATE_HEADER})

add_library(speck
        SHARED
        ${libspeck_SRCS}
        )

if (WITH_NEON)
  target_compile_definitions(speck PRIVATE "-DWITH_NEON")
endif()


if (DISABLE_SHARED)
add_library(libspeck STAIC ${libspeck_SRCS})
else (DISABLE_SHARED)
add_library(libspeck SHARED ${libspeck_SRCS})
endif (DISABLE_SHARED)

SET_TARGET_PROPERTIES(libspeck PROPERTIES OUTPUT_NAME "speck")
#SET_TARGET_PROPERTIES(libspeck PROPERTIES SOVERSION 0) # TODO
#SET_TARGET_PROPERTIES(libspeck PROPERTIES INSTALL_NAME_DIR ${LIB_INSTALL_DIR})

if (ENABLE_STATIC AND NOT DISABLE_SHARED)
add_library(libspeck_static STATIC ${libspeck_SRCS})
set_target_properties(libspeck_static PROPERTIES OUTPUT_NAME "speck")
#set_target_properties(libspeck_static PROPERTIES SOVERSION 0) # TODO
#install(TARGETS libspeck_static LIBRARY DESTINATION ${LIB_INSTALL_DIR} ARCHIVE DESTINATION ${LIB_INSTALL_DIR})
endif (ENABLE_STATIC AND NOT DISABLE_SHARED)

if (NOT (IOS OR ANDROID))
  add_executable(speck_test src/speck/speck_test.cpp)
  target_link_libraries(speck_test PUBLIC speck)
endif()

if(ENABLE_TESTING)
  enable_testing()
  add_subdirectory("test")
endif()
