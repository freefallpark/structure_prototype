cmake_minimum_required(VERSION 4.0)
project(structure_prototype)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV 4.10 REQUIRED NO_DEFAULT_PATH PATHS "/home/pkyle/neovision/nv_dependencies/install")
link_libraries(${OpenCV_LIBS})
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(example main.cpp)

find_package(GTest REQUIRED NO_DEFAULT_PATH PATHS "/home/pkyle/neovision/nv_dependencies/install")
add_executable(test test.cpp)
target_link_libraries(test PRIVATE GTest::gmock_main)


