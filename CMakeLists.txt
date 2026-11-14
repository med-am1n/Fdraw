cmake_minimum_required(VERSION 3.10)
project(Fdraw)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)


set(IMGUI_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(app
    src/main.cpp
    src/stb_image.cpp
    external/glad/src/gl.c
    ${IMGUI_SOURCES}
)

configure_file(
    ${CMAKE_SOURCE_DIR}/includes/root_directory.h.in
    ${CMAKE_BINARY_DIR}/generated/root_directory.h
)

target_include_directories(app PRIVATE
    external/glad/include
    external/glm
    external/imgui
    includes
    ${CMAKE_BINARY_DIR}/generated
)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(app PRIVATE
    glfw
    OpenGL::GL
  )
