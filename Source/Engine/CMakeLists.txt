# our name
set(PROJECT_NAME "SlateEngine")
project(${PROJECT_NAME}
        LANGUAGES C CXX
)
# if debugging
set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(${PROJECT_NAME}
        lib/GX.cpp
        lib/Application.cpp

        lib/Window.cpp
        lib/Filesystem.cpp
        lib/Timer.cpp
        lib/InputHandler.cpp

        lib/vkinfo.cpp
        lib/vkutil.cpp

        lib/PipelineBuilder.cpp
        lib/MeshGenerators.cpp

        lib/Scene.cpp
        lib/Entity.cpp

        lib/Loaders/GLTFLoader.cpp
        lib/Loaders/ShaderLoader.cpp
        lib/Loaders/ImageLoader.cpp

        lib/Resources/TextureResource.cpp
        lib/Resources/IResource.cpp
        lib/Resources/ShaderResource.cpp
        lib/Resources/MeshResource.cpp
        lib/Resources/ScriptResource.cpp

        lib/Systems/RenderSystem.cpp
        lib/Systems/ShaderSystem.cpp
        lib/Systems/TransformSystem.cpp
        lib/Systems/ISystem.cpp

        lib/CommandBuffer.cpp
        lib/GXBackend.cpp
        lib/VulkanSwapchain.cpp
        lib/VulkanImmediateCommands.cpp
        lib/VulkanStagingDevice.cpp
        lib/vkimpl.cpp
        lib/ShaderCursor.cpp
        lib/RenderPassBuilder.cpp
        lib/Bitmap.cpp
        lib/ResourceRegistry.cpp

)
# platform specific implementations
if (WIN32)
    target_sources(${PROJECT_NAME}
            PRIVATE
            lib/Platform/Windows/NamedPipe_Windows.cpp
    )
elseif (APPLE OR UNIX)
    target_sources(${PROJECT_NAME}
            PRIVATE
            lib/Platform/Unix/NamedPipe_Unix.cpp
            lib/Platform/Unix/Socket_Unix.cpp
    )
endif()

# third party / vendor libraries are installed via 3 methods
# 1. git submodules
# 2. manually cmake search
# 3. vcpkg
# should probably not have them all different but whatever for now

# git submodules
if (WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
endif()
add_subdirectory(third_party/volk)
add_subdirectory(third_party/vk-bootstrap)

# link VULKAN SDK and the also installed Slang Shading Lang Binaries
find_package(Vulkan REQUIRED COMPONENTS)
if(NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan SDK not installed.")
endif()
set(SLANG_INCLUDE_DIR "${Vulkan_INCLUDE_DIR}/slang")
set(SLANG_LIBRARY_DIR "${Vulkan_LIBRARY_DIR}")
find_library(SLANG_LIBRARY
        NAMES slang
        PATHS "${SLANG_LIBRARY_DIR}"
        REQUIRED
)
if(NOT SLANG_LIBRARY)
    message(FATAL_ERROR "Slang library not found!")
endif()
add_library(Slang::slang UNKNOWN IMPORTED)
set_target_properties(Slang::slang PROPERTIES
        IMPORTED_LOCATION "${SLANG_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SLANG_INCLUDE_DIR}"
)

# locate packages
find_package(glfw3 REQUIRED)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
find_package(EnTT REQUIRED)
find_package(Stb REQUIRED)
find_package(fmt REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
find_path(ZPP_BITS_INCLUDE_DIRS "zpp_bits.h") # header only lib

# include and link it
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${ZPP_BITS_INCLUDE_DIRS}
        ${Stb_INCLUDE_DIR}
)
target_compile_definitions(${PROJECT_NAME}
        PUBLIC
        VK_NO_PROTOTYPES # for volk to prevent vulkan1 link
)
target_link_libraries(${PROJECT_NAME}
        PUBLIC # can be found in all engine files
        volk::volk
        vk-bootstrap::vk-bootstrap
        EnTT::EnTT
        PRIVATE # can only be found in engine source files
        KTX::ktx
        Vulkan::Vulkan
        Slang::slang
        glfw
        fmt::fmt
        fastgltf::fastgltf
)