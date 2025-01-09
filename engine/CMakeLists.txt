
set(PROJECT_NAME "SlateEngine")
project(${PROJECT_NAME}
    LANGUAGES C CXX
)

add_library(${PROJECT_NAME}
        lib/Application.cpp
        lib/Window.cpp
        lib/Time.cpp
        lib/Input.cpp
        include/Slate/Ref.h

)

# locate packages
find_package(assimp REQUIRED)
find_package(EnTT REQUIRED)
find_package(glm REQUIRED)
find_package(Stb REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

add_subdirectory(third_party/vk-bootstrap)

# include and link it
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        assimp::assimp
        EnTT::EnTT
        glm::glm
        Vulkan::Vulkan
        glfw
        vk-bootstrap::vk-bootstrap
        PRIVATE
)