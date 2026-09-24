#pragma once

#include <Myve/Core/Window.h>
#include <Myve/Graphics/Vulkan/VulkanContext.h>
#include <Myve/Graphics/Vulkan/VulkanSwapchain.h>
#include <Myve/Graphics/Vulkan/UniqueVkHandle.h>
#include <Myve/Graphics/VulkanMemory.h>

#include <memory>
#include <string>

namespace myve {

struct EngineConfig {
    std::string windowTitle = "Myve Engine";
    uint32_t windowWidth = 1280;
    uint32_t windowHeight = 720;
    bool resizable = true;
    bool enableValidation = true;
};

class MyveEngine {
public:
    explicit MyveEngine(const EngineConfig& config = {});
    virtual ~MyveEngine();

    MyveEngine(const MyveEngine&) = delete;
    MyveEngine& operator=(const MyveEngine&) = delete;

    MyveEngine(MyveEngine&&) noexcept = default;
    MyveEngine& operator=(MyveEngine&&) noexcept = default;

    void run();
    void step();
    void stop() noexcept;

    [[nodiscard]] bool isRunning() const noexcept;

    [[nodiscard]] Window& getWindow() noexcept { return *m_window; }
    [[nodiscard]] const Window& getWindow() const noexcept { return *m_window; }

    [[nodiscard]] VulkanContext& getContext() noexcept { return *m_context; }
    [[nodiscard]] const VulkanContext& getContext() const noexcept { return *m_context; }

    [[nodiscard]] VulkanSwapchain& getSwapchain() noexcept { return *m_swapchain; }
    [[nodiscard]] const VulkanSwapchain& getSwapchain() const noexcept { return *m_swapchain; }

    [[nodiscard]] VulkanMemoryManager& getMemoryManager() noexcept { return *m_memory; }
    [[nodiscard]] const VulkanMemoryManager& getMemoryManager() const noexcept { return *m_memory; }

protected:
    virtual void onUpdate(float deltaTime) { (void)deltaTime; }
    virtual void onRender(VkCommandBuffer cmd, uint32_t imageIndex);

private:
    void initVulkanResources();
    void recreateSwapchain();
    void drawFrame();

    EngineConfig m_config;
    bool m_running{false};

    std::unique_ptr<Window> m_window;
    std::unique_ptr<VulkanContext> m_context;
    std::unique_ptr<VulkanSwapchain> m_swapchain;
    std::unique_ptr<VulkanMemoryManager> m_memory;

    // Recursos gerenciados via UniqueVkHandle
    UniqueVkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer{VK_NULL_HANDLE};

    UniqueVkSemaphore m_imageAvailableSemaphore;
    UniqueVkSemaphore m_renderFinishedSemaphore;
    UniqueVkFence m_inFlightFence;
};

} // namespace myve
