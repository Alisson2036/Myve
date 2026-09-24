#pragma once

#include <Myve/Core/Window.h>
#include <Myve/Graphics/Vulkan/UniqueVkHandle.h>
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

namespace myve {

class VulkanContext {
public:
    explicit VulkanContext(const Window& window, bool enableValidation = true);
    ~VulkanContext();

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    VulkanContext(VulkanContext&& other) noexcept;
    VulkanContext& operator=(VulkanContext&& other) noexcept;

    void waitIdle() const;

    [[nodiscard]] VkInstance getInstance() const noexcept { return m_instance; }
    [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const noexcept { return m_physicalDevice; }
    [[nodiscard]] VkDevice getDevice() const noexcept { return m_device; }
    [[nodiscard]] VkSurfaceKHR getSurface() const noexcept { return m_surface; }
    [[nodiscard]] VkQueue getGraphicsQueue() const noexcept { return m_graphicsQueue; }
    [[nodiscard]] uint32_t getGraphicsQueueFamily() const noexcept { return m_graphicsQueueFamily; }
    [[nodiscard]] VkQueue getPresentQueue() const noexcept { return m_presentQueue; }
    [[nodiscard]] uint32_t getPresentQueueFamily() const noexcept { return m_presentQueueFamily; }
    [[nodiscard]] const vkb::Device& getVkbDevice() const noexcept { return m_vkbDevice; }

private:
    void cleanup();

    vkb::Instance m_vkbInstance{};
    vkb::PhysicalDevice m_vkbPhysicalDevice{};
    vkb::Device m_vkbDevice{};

    VkInstance m_instance{VK_NULL_HANDLE};
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_device{VK_NULL_HANDLE};
    VkQueue m_graphicsQueue{VK_NULL_HANDLE};
    uint32_t m_graphicsQueueFamily{0};
    VkQueue m_presentQueue{VK_NULL_HANDLE};
    uint32_t m_presentQueueFamily{0};
};

} // namespace myve
