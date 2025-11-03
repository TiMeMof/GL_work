# 版本、项目名
cmake_minimum_required(VERSION 3.10)
project(HelloOpenGL CXX)

# 添加.h头文件包含路径
include_directories(${PROJECT_SOURCE_DIR}/include/glad)
include_directories(${PROJECT_SOURCE_DIR}/include/GLFW)
include_directories(${PROJECT_SOURCE_DIR}/include/glm)
include_directories(${PROJECT_SOURCE_DIR}/include/image)

# 添加.a静态库链接路径并链接glfw3库
link_directories(${PROJECT_SOURCE_DIR}/lib)
link_libraries(glad glfw3)

# 设置1.exe输出在源目录而非build中
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/)

# 可执行文件(1.exe)
add_executable(1 main.cpp)