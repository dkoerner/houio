cmake_minimum_required(VERSION 2.8)
add_definitions(-DTESTS_FILE_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}\")
add_executable( test_logger main.cpp )
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..) 
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../hougeo) 
target_link_libraries(test_logger hougeo)




add_executable( example_readwrite example_readwrite.cpp )
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/..) 
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../hougeo) 
target_link_libraries(example_readwrite hougeo)
