cmake_minimum_required(VERSION 3.14)
project(Lab02)

set(CMAKE_CXX_STANDARD 14)
set(BASE /usr/local/include)
set(GLEW_H /usr/local/Cellar/glew/2.1.0/include/GL)
set(GLFW_H /usr/local/Cellar/glfw/3.3/include/GLFW)
set(STB /Users/lyapunov/Documents/stb)
include_directories(${BASE} ${GLEW_H} ${GLFW_H} ${STB})

set(GLEW_LINK /usr/local/Cellar/glew/2.1.0/lib/libGLEW.2.1.dylib)
set(GLFW_LINK /usr/local/Cellar/glfw/3.3/lib/libglfw.3.dylib)
link_libraries(${GLEW_LINK} ${GLFW_LINK})

set(SOURCE_FILES main.cpp glad.c loader.cpp shader.hpp camera.hpp car.hpp stb_image.c)
add_executable(Lab02 ${SOURCE_FILES})
if (APPLE)
    target_link_libraries(Lab02 "-framework OpenGL")
    target_link_libraries(Lab02 "-framework GLUT")
endif()