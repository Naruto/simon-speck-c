include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/test/utils)

add_executable(speck128256ecb speck128256ecb.cpp)
target_link_libraries(speck128256ecb speck)
add_test(NAME speck128256ecb COMMAND speck128256ecb)

add_executable(speck128256ctr speck128256ctr.cpp)
target_link_libraries(speck128256ctr speck)
add_test(NAME speck128256ctr COMMAND speck128256ctr)

if(ENABLE_BENCHMARK)
    add_executable(speck128256benchmark speck128256benchmark.cpp)
    target_link_libraries(speck128256benchmark speck)
    add_test(NAME speck128256benchmark COMMAND speck128256benchmark)
endif()