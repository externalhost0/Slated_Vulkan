cmake_minimum_required(VERSION 3.28)

# global cmake vars
set(SLATE_BASE_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}")
set(SLATE_GLOBAL_VERSION "0.0.1")

# cmake settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# use custom macro/functions
include("${SLATE_BASE_FOLDER}/cmake/buildhelper.cmake")

project(Slate
        LANGUAGES C CXX
        VERSION ${SLATE_GLOBAL_VERSION}
        DESCRIPTION "Slate Editor & Engine"
)

# add defines
helper_detectOS()
helper_addDefines()

add_subdirectory("${SLATE_BASE_FOLDER}/engine")
add_subdirectory("${SLATE_BASE_FOLDER}/editor")