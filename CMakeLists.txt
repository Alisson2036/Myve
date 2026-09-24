cmake_minimum_required(VERSION 3.24)
project(Myve LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ativa a exportação dos comandos de compilação para LSPs (clangd)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Localiza o Vulkan SDK
find_package(Vulkan REQUIRED)

# Dependências externas via FetchContent
include(FetchContent)

# 1. GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# 2. vk-bootstrap
FetchContent_Declare(
    vk-bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
    GIT_TAG        v1.3.296
)

# 3. Vulkan Memory Allocator (VMA)
FetchContent_Declare(
    VulkanMemoryAllocator
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG        v3.1.0
)
set(VMA_BUILD_SAMPLE OFF CACHE BOOL "" FORCE)
set(VMA_BUILD_DOCUMENTATION OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw vk-bootstrap VulkanMemoryAllocator)

# ==============================================================================
# Biblioteca estática da Engine Myve
# ==============================================================================
set(MYVE_SOURCES
    src/Core/Exception.cpp
    src/Core/Window.cpp
    src/Graphics/Vulkan/VulkanContext.cpp
    src/Graphics/Vulkan/VulkanSwapchain.cpp
    src/Graphics/VulkanMemory.cpp
    src/MyveEngine.cpp
)

add_library(${PROJECT_NAME} STATIC ${MYVE_SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Vulkan::Vulkan
        glfw
        vk-bootstrap::vk-bootstrap
        GPUOpen::VulkanMemoryAllocator
)

# ==============================================================================
# Executável de Sandbox / Testes (linkado estaticamente à Myve)
# ==============================================================================
add_executable(Sandbox sandbox/main.cpp)

target_link_libraries(Sandbox
    PRIVATE
        ${PROJECT_NAME}
)
