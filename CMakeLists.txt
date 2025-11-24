# 版本、项目名
cmake_minimum_required(VERSION 3.10)
project(HelloOpenGL CXX)

# 添加.h头文件包含路径
include_directories(${PROJECT_SOURCE_DIR}/inc)
include_directories(${PROJECT_SOURCE_DIR}/inc/ray_tracing)
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/glad)
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/GLFW)
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/glm)
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/image)

# 添加.a静态库链接路径并链接glfw3库
link_directories(${PROJECT_SOURCE_DIR}/lib)
link_libraries(glad glfw3)

# 设置exe输出在源目录而非build中
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 可执行文件(1.exe)
add_executable(sun_earth_moon src/sun_earth_moon/sun_earth_moon.cpp)
add_executable(ray_tracing src/ray_tracing/ray_tracing.cpp)