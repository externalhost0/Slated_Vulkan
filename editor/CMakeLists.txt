
set(PROJECT_NAME "SlateEditor")
project(${PROJECT_NAME}
        LANGUAGES C CXX
)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/EditorApplication.cpp
)


target_include_directories(${PROJECT_NAME}
        PUBLIC
        SlateEngine
)
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        SlateEngine
)

