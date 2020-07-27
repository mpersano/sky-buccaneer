find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

set(GAME_SOURCES
    main.cc
    window.cc
    window.h
)

add_executable(game ${GAME_SOURCES})

target_include_directories(game
PUBLIC
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    glm
    glfw
)

target_link_libraries(game
PUBLIC
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    glm
    glfw
)
