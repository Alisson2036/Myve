#pragma once

#include <string>
#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace myve {

struct WindowConfig {
    std::string title = "Myve Engine";
    uint32_t width = 1280;
    uint32_t height = 720;
    bool resizable = false;
};

class Window {
public:
    explicit Window(const WindowConfig& config = {});
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    bool shouldClose() const noexcept;
    void pollEvents() const noexcept;

    void getFramebufferSize(int* width, int* height) const noexcept;
    bool wasResized() const noexcept { return m_framebufferResized; }
    void resetResizedFlag() noexcept { m_framebufferResized = false; }

    VkResult createSurface(VkInstance instance, VkSurfaceKHR* surface) const noexcept;

    GLFWwindow* getNativeWindow() const noexcept { return m_window; }
    uint32_t getWidth() const noexcept { return m_width; }
    uint32_t getHeight() const noexcept { return m_height; }

private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* m_window{nullptr};
    uint32_t m_width{1280};
    uint32_t m_height{720};
    bool m_framebufferResized{false};
};

} // namespace myve
