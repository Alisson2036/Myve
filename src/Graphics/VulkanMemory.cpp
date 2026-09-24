#define VMA_IMPLEMENTATION
#include <Myve/Graphics/VulkanMemory.h>
#include <Myve/Core/Exception.h>

namespace myve {

void* AllocatedBuffer::map(VmaAllocator allocator) {
    if (!allocator || allocation == VK_NULL_HANDLE) {
        _throwMsg("Tentativa de mapear AllocatedBuffer inválido ou alocador nulo.");
    }
    void* mapped = nullptr;
    VkResult res = vmaMapMemory(allocator, allocation, &mapped);
    _throwVk(res);
    return mapped;
}

void AllocatedBuffer::unmap(VmaAllocator allocator) noexcept {
    if (allocator && allocation != VK_NULL_HANDLE) {
        vmaUnmapMemory(allocator, allocation);
    }
}

VulkanMemoryManager::VulkanMemoryManager(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) {
    init(instance, physicalDevice, device);
}

VulkanMemoryManager::~VulkanMemoryManager() {
    cleanup();
}

void VulkanMemoryManager::init(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) {
    cleanup();
    m_device = device;

    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;

    VkResult res = vmaCreateAllocator(&allocatorInfo, &m_allocator);
    _throwVk(res);
}

void VulkanMemoryManager::cleanup() {
    if (m_allocator != VK_NULL_HANDLE) {
        vmaDestroyAllocator(m_allocator);
        m_allocator = VK_NULL_HANDLE;
    }
    m_device = VK_NULL_HANDLE;
}

VulkanMemoryManager::VulkanMemoryManager(VulkanMemoryManager&& other) noexcept
    : m_device(other.m_device), m_allocator(other.m_allocator) {
    other.m_device = VK_NULL_HANDLE;
    other.m_allocator = VK_NULL_HANDLE;
}

VulkanMemoryManager& VulkanMemoryManager::operator=(VulkanMemoryManager&& other) noexcept {
    if (this != &other) {
        cleanup();
        m_device = other.m_device;
        m_allocator = other.m_allocator;
        other.m_device = VK_NULL_HANDLE;
        other.m_allocator = VK_NULL_HANDLE;
    }
    return *this;
}

AllocatedBuffer VulkanMemoryManager::createBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VmaMemoryUsage memoryUsage,
    VmaAllocationCreateFlags flags
) {
    if (m_allocator == VK_NULL_HANDLE) {
        _throwMsg("VulkanMemoryManager não inicializado ao criar buffer.");
    }

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memoryUsage;
    allocInfo.flags = flags;

    AllocatedBuffer outBuffer{};
    outBuffer.size = size;

    VkResult res = vmaCreateBuffer(
        m_allocator,
        &bufferInfo,
        &allocInfo,
        &outBuffer.buffer,
        &outBuffer.allocation,
        &outBuffer.info
    );
    _throwVk(res);

    return outBuffer;
}

void VulkanMemoryManager::destroyBuffer(AllocatedBuffer& buffer) noexcept {
    if (m_allocator != VK_NULL_HANDLE && buffer.buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(m_allocator, buffer.buffer, buffer.allocation);
        buffer.buffer = VK_NULL_HANDLE;
        buffer.allocation = VK_NULL_HANDLE;
        buffer.size = 0;
    }
}

AllocatedImage VulkanMemoryManager::createImage(
    VkExtent3D extent,
    VkFormat format,
    VkImageUsageFlags usage,
    VkImageAspectFlags aspectFlags
) {
    if (m_allocator == VK_NULL_HANDLE || m_device == VK_NULL_HANDLE) {
        _throwMsg("VulkanMemoryManager não inicializado ao criar imagem.");
    }

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent = extent;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

    AllocatedImage outImage{};
    outImage.extent = extent;
    outImage.format = format;

    VkResult res = vmaCreateImage(
        m_allocator,
        &imageInfo,
        &allocInfo,
        &outImage.image,
        &outImage.allocation,
        nullptr
    );
    _throwVk(res);

    // Criação do ImageView associado
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = outImage.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    res = vkCreateImageView(m_device, &viewInfo, nullptr, &outImage.imageView);
    if (res != VK_SUCCESS) {
        vmaDestroyImage(m_allocator, outImage.image, outImage.allocation);
        _throwVk(res);
    }

    return outImage;
}

void VulkanMemoryManager::destroyImage(AllocatedImage& image) noexcept {
    if (m_device != VK_NULL_HANDLE && image.imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(m_device, image.imageView, nullptr);
        image.imageView = VK_NULL_HANDLE;
    }
    if (m_allocator != VK_NULL_HANDLE && image.image != VK_NULL_HANDLE) {
        vmaDestroyImage(m_allocator, image.image, image.allocation);
        image.image = VK_NULL_HANDLE;
        image.allocation = VK_NULL_HANDLE;
    }
}

} // namespace myve
