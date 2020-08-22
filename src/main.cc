#include "noncopyable.h"
#include "panic.h"
#include "world.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace {

class GameWindow : private NonCopyable
{
public:
    GameWindow(int width, int height, const char *title);
    ~GameWindow();

    void renderLoop();

private:
    static void sizeCallback(GLFWwindow *window, int width, int height);
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);

    void resize(int width, int height);
    void key(int key, int scancode, int action, int mode);

    GLFWwindow *m_window = nullptr;
    std::unique_ptr<World> m_world;
};

GameWindow::GameWindow(int width, int height, const char *title)
{
    glfwInit();
    glfwSetErrorCallback([](int error, const char *description) {
        panic("GLFW error %08x: %s\n", error, description);
    });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, GameWindow::keyCallback);
    glfwSetWindowSizeCallback(m_window, GameWindow::sizeCallback);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    auto error = glewInit();
    if (error != GLEW_OK) {
        panic("Failed to initialize GLEW: %s\n", glewGetErrorString(error));
    }

    if (!glewIsSupported("GL_VERSION_4_2")) {
        panic("OpenGL 4.2 not supported\n");
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(
            [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar *message,
               const void * /*user*/) { std::cerr << source << ':' << type << ':' << severity << ':' << message << '\n'; },
            nullptr);

    m_world.reset(new World);

    int windowWidth, windowHeight;
    glfwGetWindowSize(m_window, &windowWidth, &windowHeight);
    m_world->resize(windowWidth, windowHeight);
}

void GameWindow::renderLoop()
{
    double curTime = glfwGetTime();
    while (!glfwWindowShouldClose(m_window)) {
        float elapsed;
        auto now = glfwGetTime();
        elapsed = now - curTime;
        curTime = now;

        m_world->update(elapsed);
        m_world->render();

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

GameWindow::~GameWindow()
{
    m_world.reset();
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void GameWindow::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    auto *gameWindow = reinterpret_cast<GameWindow *>(glfwGetWindowUserPointer(window));
    gameWindow->key(key, scancode, action, mode);
}

void GameWindow::sizeCallback(GLFWwindow *window, int width, int height)
{
    auto *gameWindow = reinterpret_cast<GameWindow *>(glfwGetWindowUserPointer(window));
    gameWindow->resize(width, height);
}

void GameWindow::key(int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(m_window, 1);
    }
}

void GameWindow::resize(int width, int height)
{
    m_world->resize(width, height);
}

} // namespace

int main()
{
    GameWindow window(800, 400, "hello");
    window.renderLoop();
}
