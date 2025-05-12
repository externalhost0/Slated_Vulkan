macro(helper_compileGLSLShadersEXT)
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

macro(helper_compileSlangShadersEXT)
    # locates slang_c executable
    if(NOT Vulkan_SLANGC_EXECUTABLE)
        get_filename_component(_Vulkan_LIB_DIR ${Vulkan_LIBRARY} DIRECTORY)
        find_program(Vulkan_SLANGC_EXECUTABLE
                NAMES slangc
                HINTS ${_Vulkan_LIB_DIR}/../Bin
        )
    endif()

    # set shader asset dir
    set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

    # output flags
    set(_SLANG_FLAGS
            -profile spirv_1_5
            -capability spvInt64Atomics
            -target spirv
            -emit-spirv-directly
            -fvk-use-entrypoint-name
            -fvk-use-gl-layout
            -matrix-layout-column-major
            -I ${SHADER_DIR}/BuiltIn
            -O1 # optimize 0-3
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

macro(helper_compileHLSLShadersEXT)
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
