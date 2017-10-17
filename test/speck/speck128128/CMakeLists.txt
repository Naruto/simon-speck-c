include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/test/utils)

add_executable(speck128128ecb speck128128ecb.cpp)
target_link_libraries(speck128128ecb speck)
add_test(NAME speck128128ecb COMMAND speck128128ecb)

add_executable(speck128128ctr speck128128ctr.cpp)
target_link_libraries(speck128128ctr speck)
add_test(NAME speck128128ctr COMMAND speck128128ctr)

if(ENABLE_BENCHMARK)
    add_executable(speck128128benchmark speck128128benchmark.cpp)
    target_link_libraries(speck128128benchmark speck)
    add_test(NAME speck128128benchmark COMMAND speck128128benchmark)
endif()