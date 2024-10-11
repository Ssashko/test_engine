cmake_minimum_required(VERSION 3.13)
project(Engine)

include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        master
)

FetchContent_MakeAvailable(glm)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        latest
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG        master 
)
FetchContent_MakeAvailable(glad)

FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG        master
)

FetchContent_MakeAvailable(freetype)

add_library(Engine STATIC 
"graphic_manager/graphic_manager.cpp"
 "graphic_manager/graphic_resource.cpp" "engine.cpp" "collider_manager/collider_handlers.cpp" "collider_manager/collider_manager.cpp" "collider_manager/quadtree.cpp" "graphic_manager/fps_counter_renderer.cpp")

target_include_directories(Engine PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR}/include
${glad_SOURCE_DIR}/include
${glfw_SOURCE_DIR}/include
${glm_SOURCE_DIR}
)

target_link_libraries(Engine PUBLIC glad glfw glm freetype)
target_compile_features(Engine PUBLIC cxx_std_20)

set(FONT_NAME "Arial.ttf")
set(FONT_PATH_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resource/font/${FONT_NAME}")
target_compile_definitions(Engine PRIVATE FONT_NAME="${FONT_NAME}")

if (DEFINED FONT_PATH_DESTINATION)
    message(STATUS "Font destination path: ${FONT_PATH_DESTINATION}")

    add_custom_command(
        TARGET Engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${FONT_PATH_SOURCE}
        ${FONT_PATH_DESTINATION}
    )
else()
    message(WARNING "FONT_PATH_DESTINATION is not defined.")
endif()