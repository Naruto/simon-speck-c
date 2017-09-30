include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(speck128192ecb speck128192ecb.cpp)
target_link_libraries(speck128192ecb speck)
add_test(NAME speck128192ecb COMMAND speck128192ecb)

add_executable(speck128192ctr speck128192ctr.cpp)
target_link_libraries(speck128192ctr speck)
add_test(NAME speck128192ctr COMMAND speck128192ctr)

