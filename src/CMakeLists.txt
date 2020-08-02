find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

set(GAME_SOURCES
    main.cc
    mesh.cc
    shaderprogram.cc
    world.cc
    entity.cc
    transform.cc
    transformutils.cc
    renderer.cc
    shadowbuffer.cc
    camera.cc
)

add_executable(game ${GAME_SOURCES})

target_compile_features(game PUBLIC cxx_std_17)

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
