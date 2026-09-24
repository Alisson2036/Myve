#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <cstdint>

namespace myve {

struct AllocatedBuffer {
    VkBuffer buffer{VK_NULL_HANDLE};
    VmaAllocation allocation{VK_NULL_HANDLE};
    VmaAllocationInfo info{};
    VkDeviceSize size{0};

    [[nodiscard]] bool isValid() const noexcept { return buffer != VK_NULL_HANDLE; }
    void* map(VmaAllocator allocator);
    void unmap(VmaAllocator allocator) noexcept;
};

struct AllocatedImage {
    VkImage image{VK_NULL_HANDLE};
    VkImageView imageView{VK_NULL_HANDLE};
    VmaAllocation allocation{VK_NULL_HANDLE};
    VkExtent3D extent{};
    VkFormat format{VK_FORMAT_UNDEFINED};

    [[nodiscard]] bool isValid() const noexcept { return image != VK_NULL_HANDLE; }
};

class VulkanMemoryManager {
public:
    VulkanMemoryManager() = default;
    VulkanMemoryManager(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
    ~VulkanMemoryManager();

    VulkanMemoryManager(const VulkanMemoryManager&) = delete;
    VulkanMemoryManager& operator=(const VulkanMemoryManager&) = delete;

    VulkanMemoryManager(VulkanMemoryManager&& other) noexcept;
    VulkanMemoryManager& operator=(VulkanMemoryManager&& other) noexcept;

    void init(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
    void cleanup();

    AllocatedBuffer createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO,
        VmaAllocationCreateFlags flags = 0
    );
    void destroyBuffer(AllocatedBuffer& buffer) noexcept;

    AllocatedImage createImage(
        VkExtent3D extent,
        VkFormat format,
        VkImageUsageFlags usage,
        VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT
    );
    void destroyImage(AllocatedImage& image) noexcept;

    [[nodiscard]] VmaAllocator getAllocator() const noexcept { return m_allocator; }

private:
    VkDevice m_device{VK_NULL_HANDLE};
    VmaAllocator m_allocator{VK_NULL_HANDLE};
};

} // namespace myve
