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
    frustum.cc
    shadermanager.cc
    image.cc
    texture.cc
    material.cc
    materialcache.cc
    datastream.cc
    level.cc
    octree.cc
    geometryutils.cc
    gameobject.cc
    player.cc
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
    glm
    glfw
    spdlog
    stb
)

add_custom_command(TARGET game
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/assets" "${CMAKE_CURRENT_BINARY_DIR}/assets"
)
