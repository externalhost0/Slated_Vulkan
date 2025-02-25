include(FetchContent)

macro(helper_detectOS)
    if (WIN32)
        add_compile_definitions(SLATE_OS_WINDOWS)
        set(CMAKE_INSTALL_BINDIR ".")
        set(CMAKE_INSTALL_LIBDIR ".")
        set(PLUGINS_INSTALL_LOCATION "plugins")
        add_compile_definitions(UNICODE)
    elseif (APPLE)
        add_compile_definitions(SLATE_OS_MACOS)
        set(CMAKE_INSTALL_BINDIR ".")
        set(CMAKE_INSTALL_LIBDIR ".")
        set(PLUGINS_INSTALL_LOCATION "plugins")
        enable_language(OBJC)
        enable_language(OBJCXX)
    elseif (EMSCRIPTEN)
        add_compile_definitions(SLATE_OS_WEB)
    elseif (UNIX AND NOT APPLE)
        add_compile_definitions(SLATE_OS_LINUX)
        if (BSD AND BSD STREQUAL "FreeBSD")
            add_compile_definitions(SLATE_OS_FREEBSD)
        endif()
        include(GNUInstallDirs)
    else ()
        message(FATAL_ERROR "Unknown / unsupported system!")
    endif()
endmacro()

macro(helper_addGlobalDefines)
    if (NOT SLATE_GLOBAL_VERSION)
        message(FATAL_ERROR "SLATE_GLOBAL_VERSION is not defined")
    endif ()

    set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} -DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR} -DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH} ")

    set(SLATE_VERSION_STRING ${SLATE_GLOBAL_VERSION})
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        set(SLATE_VERSION_STRING ${SLATE_VERSION_STRING})
        add_compile_definitions(NDEBUG)
    elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(SLATE_VERSION_STRING ${SLATE_VERSION_STRING}-Debug)
        add_compile_definitions(SLATE_DEBUG)
    endif ()

    if (PROJECT_ENABLE_STD_ASSERTS)
        add_compile_definitions(_GLIBCXX_DEBUG _GLIBCXX_VERBOSE)
    endif()
endmacro()

macro(helper_addExternalDefines)
    # GLN
    add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
    add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)
    # IMGUI
    add_compile_definitions(IMGUI_DISABLE_OBSOLETE_FUNCTIONS)
endmacro()

macro(helper_compileGLSLShaders)
    set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

    file(GLOB SHADERS
            ${SHADER_DIR}/*.vert
            ${SHADER_DIR}/*.frag
            ${SHADER_DIR}/*.comp
            ${SHADER_DIR}/*.geom
            ${SHADER_DIR}/*.tesc
            ${SHADER_DIR}/*.tese
            ${SHADER_DIR}/*.mesh
            ${SHADER_DIR}/*.task
            ${SHADER_DIR}/*.rgen
            ${SHADER_DIR}/*.rchit
            ${SHADER_DIR}/*.rmiss
    )
    foreach (SHADER IN LISTS SHADERS)
        get_filename_component(FILENAME ${SHADER} NAME)
        add_custom_command(
                OUTPUT ${SHADER_DIR}/compiled_shaders/${FILENAME}.spv
                COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SHADER_DIR}/compiled_shaders/${FILENAME}.spv
                DEPENDS ${SHADER}
                COMMENT "Compiling GLSL: ${FILENAME}"
        )
        list(APPEND SPV_SHADERS ${SHADER_DIR}/compiled_shaders/${FILENAME}.spv)
    endforeach ()

    add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
endmacro()

macro(helper_compileSlangShaders)
    # set shader asset dir
    set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

    # output flags
    set(_SLANG_FLAGS
            -profile spirv_1_6
            -capability spvInt64Atomics
            -target spirv
            -emit-spirv-directly
            -fvk-use-entrypoint-name
            -fvk-use-gl-layout
            -matrix-layout-column-major
            -O2 # optimize 0-3
    )

    # gather .slang shaders and compile
    file(GLOB SHADERS
            ${SHADER_DIR}/*.slang
    )
    foreach (SHADER IN LISTS SHADERS)
        get_filename_component(FILENAME ${SHADER} NAME)
        add_custom_command(
                OUTPUT ${SHADER_DIR}/compiled_shaders/${FILENAME}.spv
                COMMAND ${Vulkan_SLANGC_EXECUTABLE} ${_SLANG_FLAGS} ${SHADER} -o ${SHADER_DIR}/compiled_shaders/${FILENAME}.spv -reflection-json ${SHADER_DIR}/compiled_shaders/reflection/${FILENAME}.json
                DEPENDS ${SHADER}
                COMMENT "Compiling Slang shader: ${FILENAME}"
        )
        list(APPEND SPV_SHADERS ${SHADER_DIR}/compiled_shaders/${FILENAME}.spv)
    endforeach ()

    add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
endmacro()

macro(helper_compileHLSLShaders)
    set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

    file(GLOB SHADERS
            ${SHADER_DIR}/*.hlsl
    )
    foreach (SHADER IN LISTS SHADERS)
        get_filename_component(FILENAME ${SHADER} NAME)
        add_custom_command(
                OUTPUT ${SHADER_DIR}/compiled_shaders/${FILENAME}.spv
                COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SHADER_DIR}/compiled_shaders/${FILENAME}.spv
                DEPENDS ${SHADER}
                COMMENT "Compiling HLSL: ${FILENAME}"
        )
        list(APPEND SPV_SHADERS ${SHADER_DIR}/compiled_shaders/${FILENAME}.spv)
    endforeach ()

    add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
endmacro()