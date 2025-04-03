
set(PROJECT_NAME "SlateEngine")
project(${PROJECT_NAME}
        LANGUAGES C CXX
)

add_library(${PROJECT_NAME}
        lib/IApplication.cpp
        lib/Window.cpp
        lib/Filesystem.cpp

        lib/vkinfo.cpp
        lib/vkutil.cpp
        lib/vkdescriptor.cpp
        lib/PipelineBuilder.cpp
        lib/Scene.cpp
        lib/MeshGenerators.cpp
        lib/Loaders/GLTFLoader.cpp
        lib/ShaderPass.cpp
        lib/Entity.cpp
        lib/Loaders/ShaderLoader.cpp

        lib/Resources/IResource.cpp
        lib/Resources/ShaderResource.cpp
        lib/Resources/MeshResource.cpp
        lib/Resources/ScriptResource.cpp
        lib/RenderEngineStartup.cpp
        lib/Systems/InputSystem.cpp
        lib/Systems/TimeSystem.cpp
        lib/Systems/WindowSystem.cpp
        lib/Systems/RenderSystem.cpp
        lib/RenderEngineOps.cpp
        include/Slate/Systems/ShaderSystem.h
        lib/Systems/ShaderSystem.cpp
        include/Slate/UUID.h
        include/Slate/FastSTD.h
)

# git submodules
add_subdirectory(third_party/volk)
add_subdirectory(third_party/vk-bootstrap)
add_subdirectory(third_party/SPIRV-Cross)

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
    message(FATAL_ERROR "Slang Lib not found!")
endif()


find_package(EnTT REQUIRED)
find_package(Stb REQUIRED)
find_package(fmt REQUIRED)
find_package(fastgltf CONFIG REQUIRED)



if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Werror
            -Wctad-maybe-unsupported
    )
endif()

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
        Vulkan::Vulkan
        glfw
        fmt::fmt
        fastgltf::fastgltf
)