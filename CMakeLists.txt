cmake_minimum_required(VERSION 3.16)
project(DigitalSignalGeneratorCpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_EXAMPLES "Build example app" ON)

include(FetchContent)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.89.14
)
FetchContent_Declare(
  implot
  GIT_REPOSITORY https://github.com/epezent/implot.git
  GIT_TAG v0.14
)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(imgui implot glfw)

# ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# ImPlot sources
set(IMPLOT_SOURCES
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
)

add_library(imgui_backend STATIC ${IMGUI_SOURCES} ${IMPLOT_SOURCES})
target_include_directories(imgui_backend PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${implot_SOURCE_DIR}
)

# Add glfw
add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR} EXCLUDE_FROM_ALL)
find_package(OpenGL REQUIRED)

# Our sources
add_executable(DigitalSignalGeneratorCpp
    src/main.cpp
    src/DigitalSignalGenerator.cpp
    src/DigitalSignalGenerator.hpp
)

target_include_directories(DigitalSignalGeneratorCpp PRIVATE src)
target_link_libraries(DigitalSignalGeneratorCpp PRIVATE imgui_backend glfw OpenGL::GL)

if (APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    target_link_libraries(DigitalSignalGeneratorCpp PRIVATE ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${CORE_FOUNDATION_LIBRARY})
endif()

# Compiler flags (nice defaults)
if(MSVC)
  target_compile_options(DigitalSignalGeneratorCpp PRIVATE /W4 /permissive-)
else()
  target_compile_options(DigitalSignalGeneratorCpp PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()