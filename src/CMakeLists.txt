find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PNG REQUIRED)

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
    frustum.cc
    shadermanager.cc
    image.cc
    texture.cc
    material.cc
    materialcache.cc
    datastream.cc
    level.cc
)

add_executable(game ${GAME_SOURCES})

target_compile_features(game PUBLIC cxx_std_17)

target_include_directories(game
PUBLIC
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
)

target_link_libraries(game
PUBLIC
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
	${PNG_LIBRARIES}
    glm
    glfw
    spdlog
)
