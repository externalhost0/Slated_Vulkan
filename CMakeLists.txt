cmake_minimum_required(VERSION 3.28)

# global cmake vars
set(SLATED_BASE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}")
set(SLATED_GLOBAL_VERSION "0.0.1")

# cmake settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# use custom macro/functions
include("${SLATED_BASE_FOLDER}/Source/Engine/cmake/buildhelper.cmake")

project(Slated
        LANGUAGES C CXX
        VERSION ${SLATED_GLOBAL_VERSION}
        DESCRIPTION "Slate Editor & Engine"
)

# add defines
helper_detectOS()
helper_addGlobalDefines()
helper_addExternalDefines()

add_subdirectory("${SLATED_BASE_FOLDER}/Source/Engine")
add_subdirectory("${SLATED_BASE_FOLDER}/Source/Boilerplate")
#
## boilerplate project
#add_executable(Slate
#        ${SLATE_BASE_FOLDER}/engine/boilerplate/main.cpp
#)
#target_include_directories(Slate
#        PUBLIC
#        SlateEngine
#)
#target_link_libraries(Slate
#        PUBLIC
#        SlateEngine
#)
#add_subdirectory("${SLATE_BASE_FOLDER}/Editor")