
set(PROJECT_NAME "SlateEngine")
project(${PROJECT_NAME}
        LANGUAGES C CXX
)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(${PROJECT_NAME}
        lib/GX.cpp
        lib/IApplication.cpp

        lib/Window.cpp
        lib/WindowService.cpp
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
        lib/CommandPool.cpp
        lib/GXBackbone.cpp
        lib/VulkanSwapchain.cpp
        lib/VulkanImmediateCommands.cpp
        lib/VulkanStagingDevice.cpp
        lib/vkimpl.cpp
        lib/ShaderCursor.cpp
        lib/RenderPassBuilder.cpp
        lib/Bitmap.cpp
        lib/ScriptEngine.cpp
)

# git submodules
add_subdirectory(third_party/volk)
add_subdirectory(third_party/vk-bootstrap)

# locate packages
find_package(glfw3 REQUIRED)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

find_package(Vulkan REQUIRED COMPONENTS)
if(NOT VULKAN_FOUND)
    message(FATAL_ERROR "Vulkan SDK not installed.")
endif()
# good

# bad
get_filename_component(_Vulkan_LIB_DIR ${Vulkan_LIBRARY} DIRECTORY)

find_library(SLANG_LIB
        NAMES slang
        HINTS ${_Vulkan_LIB_DIR}
)
if(NOT SLANG_LIB)
    message(FATAL_ERROR "Slang library not found!")
endif()


find_package(EnTT REQUIRED)
find_package(Stb REQUIRED)
find_package(fmt REQUIRED)
find_package(fastgltf CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)


# include and link it
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
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
        ${SLANG_LIB} # i really need a cmake master to help me organize libraries and their target installation

        KTX::ktx
        Vulkan::Vulkan
        glfw
        fmt::fmt
        fastgltf::fastgltf
)