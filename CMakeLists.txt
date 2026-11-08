cmake_minimum_required(VERSION 3.16)
project(AquaEngine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenGL REQUIRED)

add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

set(SDL_TESTS OFF)
set(SDL_EXAMPLES OFF)
add_subdirectory(external/sdl)

add_subdirectory(external/cimgui)
target_include_directories(cimgui PUBLIC
 external/cimgui
 external/cimgui/generator/output
 external/cimgui/imgui
 external/cimgui/imgui/backends
)

add_subdirectory(external/cglm)

add_library(
 ${PROJECT_NAME}
 src/aqua.c

 src/event/event.c
 src/tick/tick.c

 src/utils/file/file.c

 src/gfx/window/window.c

 src/gfx/shader/shader.c
 src/gfx/texture/texture.c
 src/gfx/mesh/mesh.c
 src/gfx/renderer/resource-manager.c
 src/gfx/material/material.c
 src/gfx/material/material-manager.c
 src/gfx/renderer/renderer.c
 src/gfx/model/model.c
 src/gfx/camera/camera.c

 src/fly-camera/fly-camera.c

 src/input/input.c

 external/cimgui/imgui/backends/imgui_impl_sdl3.cpp
 external/cimgui/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(
 ${PROJECT_NAME} PUBLIC
 src
 external/cgltf/include
 external/stb_image/include
)

target_compile_definitions(${PROJECT_NAME} PUBLIC -DCIMGUI_USE_OPENGL3 -DCIMGUI_USE_SDL3)

target_link_libraries(${PROJECT_NAME} PUBLIC glad SDL3::SDL3 cimgui cglm)

target_compile_options(${PROJECT_NAME} PRIVATE
 -Wall -Wextra -Wpedantic
 -Wconversion -Wshadow -Wstrict-prototypes -Wformat=2 -Wnull-dereference
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
 $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
)

# Built-in shaders are loaded at runtime relative to the working directory, which must be the
# root of the top-level project (the game). Computed so the engine can live anywhere inside it.
file(RELATIVE_PATH AQUA_ENGINE_SHADER_DIRECTORY ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders)
target_compile_definitions(${PROJECT_NAME} PRIVATE
 AQUA_ENGINE_SHADER_DIRECTORY="${AQUA_ENGINE_SHADER_DIRECTORY}"
)
