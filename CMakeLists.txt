cmake_minimum_required(VERSION 3.5)
project(simon_speck)

option(ENABLE_STATIC "Build a static version of the libraries?" OFF)
option(DISABLE_SHARED "Do not build a shared version of the libraries?" OFF)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

set(libspeck_SRCS Speck.c)
set(libspeck_HEADER Speck.h)

add_library(speck
        SHARED
        ${libspeck_SRCS}
        )


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

add_executable(simon_speck main.cpp)
target_link_libraries(simon_speck PUBLIC speck)
