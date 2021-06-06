#include "inputstate.h"
#include "noncopyable.h"
#include "panic.h"
#include "world.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <iostream>

namespace {

const std::string glDebugSourceToString(GLenum type)
{
    switch (type) {
    case GL_DEBUG_SOURCE_API:
        return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        return "WINDOW_SYSTEM";
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        return "SHADER_COMPILER";
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        return "THIRD_PARTY";
    case GL_DEBUG_SOURCE_APPLICATION:
        return "APPLICATION";
    case GL_DEBUG_SOURCE_OTHER:
        return "OTHER";
    default:
        return std::to_string(static_cast<int>(type));
    }
}

const std::string glDebugTypeToString(GLenum type)
{
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        return "ERROR";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        return "DEPRECATED_BEHAVIOR";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        return "UNDEFINED_BEHAVIOR";
    case GL_DEBUG_TYPE_PORTABILITY:
        return "PORTABILITY";
    case GL_DEBUG_TYPE_PERFORMANCE:
        return "PERFORMANCE";
    case GL_DEBUG_TYPE_MARKER:
        return "MARKER";
    case GL_DEBUG_TYPE_PUSH_GROUP:
        return "PUSH_GROUP";
    case GL_DEBUG_TYPE_POP_GROUP:
        return "POP_GROUP";
    case GL_DEBUG_TYPE_OTHER:
        return "OTHER";
    default:
        return std::to_string(static_cast<int>(type));
    }
}

const std::string glDebugSeverityToString(GLenum severity)
{
    switch (severity) {
    case GL_DEBUG_SEVERITY_LOW:
        return "LOW";
    case GL_DEBUG_SEVERITY_MEDIUM:
        return "MEDIUM";
    case GL_DEBUG_SEVERITY_HIGH:
        return "HIGH";
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        return "NOTIFICATION";
    default:
        return std::to_string(static_cast<int>(severity));
    }
}

enum class DebugMessageSeverity : unsigned {
    None = 0,
    Low = 1 << 0,
    Medium = 1 << 1,
    High = 1 << 2,
    Notification = 1 << 3,
    All = Low | Medium | High | Notification
};

constexpr DebugMessageSeverity operator|(DebugMessageSeverity x, DebugMessageSeverity y)
{
    using UT = typename std::underlying_type_t<DebugMessageSeverity>;
    return static_cast<DebugMessageSeverity>(static_cast<UT>(x) | static_cast<UT>(y));
}

constexpr DebugMessageSeverity operator&(DebugMessageSeverity x, DebugMessageSeverity y)
{
    using UT = typename std::underlying_type_t<DebugMessageSeverity>;
    return static_cast<DebugMessageSeverity>(static_cast<UT>(x) & static_cast<UT>(y));
}

DebugMessageSeverity &operator|=(DebugMessageSeverity &x, DebugMessageSeverity y)
{
    return x = x | y;
}

DebugMessageSeverity &operator&=(DebugMessageSeverity &x, DebugMessageSeverity y)
{
    return x = x & y;
}

class GameWindow : private NonCopyable
{
public:
    GameWindow(int width, int height, const char *title);
    ~GameWindow();

    void enableGLDebug(DebugMessageSeverity severityMask);
    void renderLoop();

private:
    static void sizeCallback(GLFWwindow *window, int width, int height);
    static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
    void debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, std::string_view message) const;

    void resize(int width, int height);
    void key(int key, int scancode, int action, int mode);

    GLFWwindow *m_window = nullptr;
    std::unique_ptr<World> m_world;
    InputState m_inputState = InputState::None;
    DebugMessageSeverity m_severityMask = DebugMessageSeverity::None;
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
    glfwWindowHint(GLFW_SAMPLES, 16);
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

    m_world.reset(new World);

    int windowWidth, windowHeight;
    glfwGetWindowSize(m_window, &windowWidth, &windowHeight);
    m_world->resize(windowWidth, windowHeight);
}

void GameWindow::enableGLDebug(DebugMessageSeverity severityMask)
{
    m_severityMask = severityMask;
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(
            [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user) {
                reinterpret_cast<const GameWindow *>(user)->debugMessage(source, type, id, severity, std::string_view(message, length));
            },
            this);
}

void GameWindow::debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, std::string_view message) const
{
    const auto severityBit = [severity] {
        switch (severity) {
        case GL_DEBUG_SEVERITY_LOW:
            return DebugMessageSeverity::Low;
        case GL_DEBUG_SEVERITY_MEDIUM:
            return DebugMessageSeverity::Medium;
        case GL_DEBUG_SEVERITY_HIGH:
            return DebugMessageSeverity::High;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            return DebugMessageSeverity::Notification;
        default:
            break;
        }
        return DebugMessageSeverity::None;
    }();
    if ((m_severityMask & severityBit) == DebugMessageSeverity::None)
        return;
    spdlog::info("GL debug (source={}, type={}, severity={}): {}", glDebugSourceToString(source), glDebugTypeToString(type), glDebugSeverityToString(severity), message);
}

void GameWindow::renderLoop()
{
    double curTime = glfwGetTime();
    while (!glfwWindowShouldClose(m_window)) {
        float elapsed;
        auto now = glfwGetTime();
        elapsed = now - curTime;
        curTime = now;

        m_world->update(m_inputState, elapsed);
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
    switch (key) {
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS)
            glfwSetWindowShouldClose(m_window, 1);
        break;
#define UPDATE_INPUT_STATE(key, state)          \
    case key:                                   \
        if (action == GLFW_PRESS)               \
            m_inputState |= InputState::state;  \
        else if (action == GLFW_RELEASE)        \
            m_inputState &= ~InputState::state; \
        break;
        UPDATE_INPUT_STATE(GLFW_KEY_LEFT, Left)
        UPDATE_INPUT_STATE(GLFW_KEY_RIGHT, Right)
        UPDATE_INPUT_STATE(GLFW_KEY_UP, Up)
        UPDATE_INPUT_STATE(GLFW_KEY_DOWN, Down)
        UPDATE_INPUT_STATE(GLFW_KEY_A, Forward)
        UPDATE_INPUT_STATE(GLFW_KEY_Z, Reverse)
        UPDATE_INPUT_STATE(GLFW_KEY_SPACE, Fire)
        UPDATE_INPUT_STATE(GLFW_KEY_TAB, ToggleView)
#undef UPDATE_INPUT_STATE
    default:
        break;
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
    window.enableGLDebug(DebugMessageSeverity::Medium | DebugMessageSeverity::High);
    window.renderLoop();
}
