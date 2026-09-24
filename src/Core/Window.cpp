#include <Myve/Core/Window.h>
#include <Myve/Core/Exception.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace myve {

namespace {
    int g_windowCount = 0;

    void ensureGlfwInit() {
        if (g_windowCount == 0) {
            if (!glfwInit()) {
                _throwMsg("Falha ao inicializar o GLFW.");
            }
        }
        ++g_windowCount;
    }

    void releaseGlfw() {
        --g_windowCount;
        if (g_windowCount <= 0) {
            glfwTerminate();
            g_windowCount = 0;
        }
    }
} // namespace

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self) {
        self->m_framebufferResized = true;
        self->m_width = static_cast<uint32_t>(width);
        self->m_height = static_cast<uint32_t>(height);
    }
}

Window::Window(const WindowConfig& config)
    : m_width(config.width), m_height(config.height) {
    ensureGlfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

    m_window = glfwCreateWindow(
        static_cast<int>(m_width),
        static_cast<int>(m_height),
        config.title.c_str(),
        nullptr,
        nullptr
    );

    if (!m_window) {
        releaseGlfw();
        _throwMsg("Falha ao criar janela GLFW.");
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}

Window::~Window() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        releaseGlfw();
    }
}

Window::Window(Window&& other) noexcept
    : m_window(other.m_window),
      m_width(other.m_width),
      m_height(other.m_height),
      m_framebufferResized(other.m_framebufferResized) {
    other.m_window = nullptr;
    if (m_window) {
        glfwSetWindowUserPointer(m_window, this);
    }
}

Window& Window::operator=(Window&& other) noexcept {
    if (this != &other) {
        if (m_window) {
            glfwDestroyWindow(m_window);
            releaseGlfw();
        }
        m_window = other.m_window;
        m_width = other.m_width;
        m_height = other.m_height;
        m_framebufferResized = other.m_framebufferResized;
        other.m_window = nullptr;

        if (m_window) {
            glfwSetWindowUserPointer(m_window, this);
        }
    }
    return *this;
}

bool Window::shouldClose() const noexcept {
    return m_window ? glfwWindowShouldClose(m_window) : true;
}

void Window::pollEvents() const noexcept {
    glfwPollEvents();
}

void Window::getFramebufferSize(int* width, int* height) const noexcept {
    if (m_window) {
        glfwGetFramebufferSize(m_window, width, height);
    } else {
        if (width) *width = 0;
        if (height) *height = 0;
    }
}

VkResult Window::createSurface(VkInstance instance, VkSurfaceKHR* surface) const noexcept {
    if (!m_window || instance == VK_NULL_HANDLE || !surface) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return glfwCreateWindowSurface(instance, m_window, nullptr, surface);
}

} // namespace myve
