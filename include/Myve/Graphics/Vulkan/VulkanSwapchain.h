#pragma once

#include <Myve/Graphics/Vulkan/VulkanContext.h>
#include <Myve/Graphics/Vulkan/UniqueVkHandle.h>
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>
#include <vector>

namespace myve {

class VulkanSwapchain {
public:
    VulkanSwapchain(const VulkanContext& context, uint32_t width, uint32_t height);
    ~VulkanSwapchain();

    VulkanSwapchain(const VulkanSwapchain&) = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

    VulkanSwapchain(VulkanSwapchain&& other) noexcept;
    VulkanSwapchain& operator=(VulkanSwapchain&& other) noexcept;

    void recreate(uint32_t width, uint32_t height);

    VkResult acquireNextImage(
        VkSemaphore imageAvailableSemaphore,
        VkFence fence,
        uint32_t* imageIndex,
        uint64_t timeoutNs = UINT64_MAX
    ) const noexcept;

    [[nodiscard]] VkSwapchainKHR getSwapchain() const noexcept { return m_swapchain; }
    [[nodiscard]] VkExtent2D getExtent() const noexcept { return m_extent; }
    [[nodiscard]] VkFormat getImageFormat() const noexcept { return m_imageFormat; }
    [[nodiscard]] const std::vector<VkImage>& getImages() const noexcept { return m_images; }
    [[nodiscard]] const std::vector<VkImageView>& getImageViews() const noexcept { return m_imageViews; }
    [[nodiscard]] size_t getImageCount() const noexcept { return m_images.size(); }

private:
    void init(uint32_t width, uint32_t height);
    void cleanup();

    const VulkanContext* m_context{nullptr};
    vkb::Swapchain m_vkbSwapchain{};
    VkSwapchainKHR m_swapchain{VK_NULL_HANDLE};
    VkExtent2D m_extent{};
    VkFormat m_imageFormat{VK_FORMAT_UNDEFINED};

    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
};

} // namespace myve
