cmake_minimum_required(VERSION 3.27)
project(raytracing)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8 # Specify the version you need
)

FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    fetch_vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG        v1.3.295 #suggest using a tag so the library doesn't update whenever new commits are pushed to a branch
)
FetchContent_MakeAvailable(fetch_vk_bootstrap)

# Find the required packages
find_package(Vulkan REQUIRED)

#link_libraries(-fsanitize=address)
#add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)

include_directories(${Vulkan_INCLUDE_DIRS})

set(SHADER_SOURCE_DIR "shaders")
set(SHADER_BINARY_DIR "shaders")
set(GLSL_SOURCE_FILES "")

set(SOURCE_FILES
        src/main.cpp
        src/window.h
        src/diagnostics.h
        src/diagnostics.cpp
        src/Singleton.h
        src/window.cpp
        src/vulkan/instance.h
        src/vulkan/instance.cpp
        src/vulkan/vma.cpp
        src/vulkan/vk_exception.h
        src/vulkan/vk_exception.cpp
        src/vulkan/vkb_raii.h
        src/vulkan/vkb_raii.cpp
        src/vulkan/surface.h
        src/vulkan/surface.cpp
)

foreach (GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -c --target-env=vulkan1.2 -o ${SPIRV}
            DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach (GLSL)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_BINARY_FILES}
)

# add glm
FetchContent_Declare(
        glm
        URL https://github.com/g-truc/glm/releases/download/1.0.1/glm-1.0.1-light.zip
        DOWNLOAD_NO_PROGRESS ON
        DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/downloads
)

FetchContent_GetProperties(glm)
if (NOT glm_POPULATED)
    FetchContent_Populate(glm)
endif ()

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${GLSL_SOURCE_FILES})

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources ${CMAKE_BINARY_DIR}/resources
)

add_dependencies(${PROJECT_NAME} Shaders)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${glm_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} glfw vk-bootstrap::vk-bootstrap)
 
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GPUOpen::VulkanMemoryAllocator)
