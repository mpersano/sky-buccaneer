#include "noncopyable.h"
#include "panic.h"
#include "world.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace {

class Window : private NonCopyable
{
public:
    Window(int width, int height, const char *title);
    ~Window();

    int width() const { return m_width; }
    int height() const { return m_height; }
    operator GLFWwindow *() const { return m_window; }

private:
    int m_width;
    int m_height;
    GLFWwindow *m_window;
};

Window::Window(int width, int height, const char *title)
    : m_width(width)
    , m_height(height)
{
    glfwInit();
    glfwSetErrorCallback([](int error, const char *description) {
        panic("GLFW error %08x: %s\n", error, description);
    });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

}

int main()
{
    Window window(800, 400, "hello");

    glewInit();

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(
            [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar *message,
               const void * /*user*/) { std::cerr << source << ':' << type << ':' << severity << ':' << message << '\n'; },
            nullptr);

    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mode) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
    });

    {
        World world;
        world.resize(window.width(), window.height());

        double curTime = glfwGetTime();
        while (!glfwWindowShouldClose(window)) {
            float elapsed;
            auto now = glfwGetTime();
            elapsed = now - curTime;
            curTime = now;

            world.update(elapsed);
            world.render();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
}
