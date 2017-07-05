include_directories(${PROJECT_SOURCE_DIR}/src/speck)
add_executable(check_algorithm_128128 check_algorithm_128128.c)
target_link_libraries(check_algorithm_128128 speck)

add_test(NAME check_algorithm_128128 COMMAND "./check_algorithm_128128")

