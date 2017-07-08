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

# ARM NEON
if(ENABLE_NEON)
  if(ARM OR AARCH64 OR IOS)
    set(WITH_NEON ON)
  endif()
endif()

if(WITH_NEON)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=softfp -mfpu=neon")
  if(IOS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
  endif()
endif()

if(WITH_NEON)
  set(libspeck_SRCS src/speck/speck_neon.c)
else()
  set(libspeck_SRCS src/speck/speck.c)
endif()
if(WIN32)
  set(libspeck_SRCS ${libspeck_SRCS} src/speck/dllmain.c)
endif(WIN32)
set(libspeck_HEADER src/speck/speck.h src/speck/speck_private.h)

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
