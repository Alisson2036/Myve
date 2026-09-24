#pragma once

#include <vulkan/vulkan.h>
#include <utility>
#include <cstddef>

namespace myve {

template <typename Handle, typename Parent = VkDevice>
struct DefaultVkDeleter;

// Especializações para objetos associados a VkDevice
template <>
struct DefaultVkDeleter<VkSemaphore, VkDevice> {
    void operator()(VkDevice device, VkSemaphore handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroySemaphore(device, handle, nullptr);
        }
    }
};

template <>
struct DefaultVkDeleter<VkFence, VkDevice> {
    void operator()(VkDevice device, VkFence handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroyFence(device, handle, nullptr);
        }
    }
};

template <>
struct DefaultVkDeleter<VkCommandPool, VkDevice> {
    void operator()(VkDevice device, VkCommandPool handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device, handle, nullptr);
        }
    }
};

template <>
struct DefaultVkDeleter<VkImageView, VkDevice> {
    void operator()(VkDevice device, VkImageView handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroyImageView(device, handle, nullptr);
        }
    }
};

template <>
struct DefaultVkDeleter<VkImage, VkDevice> {
    void operator()(VkDevice device, VkImage handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroyImage(device, handle, nullptr);
        }
    }
};

template <>
struct DefaultVkDeleter<VkBuffer, VkDevice> {
    void operator()(VkDevice device, VkBuffer handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroyBuffer(device, handle, nullptr);
        }
    }
};

template <>
struct DefaultVkDeleter<VkShaderModule, VkDevice> {
    void operator()(VkDevice device, VkShaderModule handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device, handle, nullptr);
        }
    }
};

template <>
struct DefaultVkDeleter<VkPipeline, VkDevice> {
    void operator()(VkDevice device, VkPipeline handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroyPipeline(device, handle, nullptr);
        }
    }
};

template <>
struct DefaultVkDeleter<VkPipelineLayout, VkDevice> {
    void operator()(VkDevice device, VkPipelineLayout handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device, handle, nullptr);
        }
    }
};

template <>
struct DefaultVkDeleter<VkRenderPass, VkDevice> {
    void operator()(VkDevice device, VkRenderPass handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device, handle, nullptr);
        }
    }
};

template <>
struct DefaultVkDeleter<VkFramebuffer, VkDevice> {
    void operator()(VkDevice device, VkFramebuffer handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device, handle, nullptr);
        }
    }
};

template <>
struct DefaultVkDeleter<VkSampler, VkDevice> {
    void operator()(VkDevice device, VkSampler handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroySampler(device, handle, nullptr);
        }
    }
};

template <>
struct DefaultVkDeleter<VkDescriptorSetLayout, VkDevice> {
    void operator()(VkDevice device, VkDescriptorSetLayout handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device, handle, nullptr);
        }
    }
};

template <>
struct DefaultVkDeleter<VkDescriptorPool, VkDevice> {
    void operator()(VkDevice device, VkDescriptorPool handle) const noexcept {
        if (device != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device, handle, nullptr);
        }
    }
};

// Especializações para objetos associados a VkInstance
template <>
struct DefaultVkDeleter<VkSurfaceKHR, VkInstance> {
    void operator()(VkInstance instance, VkSurfaceKHR handle) const noexcept {
        if (instance != VK_NULL_HANDLE && handle != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance, handle, nullptr);
        }
    }
};

/**
 * @brief Gerenciador RAII move-only de longa duração para handles Vulkan.
 */
template <typename Handle, typename Parent = VkDevice, typename Deleter = DefaultVkDeleter<Handle, Parent>>
class UniqueVkHandle {
public:
    constexpr UniqueVkHandle() noexcept : m_handle(VK_NULL_HANDLE), m_parent(VK_NULL_HANDLE) {}
    constexpr UniqueVkHandle(std::nullptr_t) noexcept : m_handle(VK_NULL_HANDLE), m_parent(VK_NULL_HANDLE) {}

    UniqueVkHandle(Parent parent, Handle handle) noexcept
        : m_handle(handle), m_parent(parent) {}

    ~UniqueVkHandle() noexcept {
        reset();
    }

    // Move-only semantics
    UniqueVkHandle(const UniqueVkHandle&) = delete;
    UniqueVkHandle& operator=(const UniqueVkHandle&) = delete;

    UniqueVkHandle(UniqueVkHandle&& other) noexcept
        : m_handle(other.m_handle), m_parent(other.m_parent) {
        other.m_handle = VK_NULL_HANDLE;
        other.m_parent = VK_NULL_HANDLE;
    }

    UniqueVkHandle& operator=(UniqueVkHandle&& other) noexcept {
        if (this != &other) {
            reset();
            m_handle = other.m_handle;
            m_parent = other.m_parent;
            other.m_handle = VK_NULL_HANDLE;
            other.m_parent = VK_NULL_HANDLE;
        }
        return *this;
    }

    void reset(Parent newParent = VK_NULL_HANDLE, Handle newHandle = VK_NULL_HANDLE) noexcept {
        if (m_handle != VK_NULL_HANDLE) {
            Deleter{}(m_parent, m_handle);
        }
        m_parent = newParent;
        m_handle = newHandle;
    }

    [[nodiscard]] Handle release() noexcept {
        Handle released = m_handle;
        m_handle = VK_NULL_HANDLE;
        m_parent = VK_NULL_HANDLE;
        return released;
    }

    /**
     * @brief Prepara o handle para ser preenchido por funções de criação da Vulkan (ex: vkCreate*(..., &handle)).
     * Reseta qualquer handle anterior antes de retornar o ponteiro.
     */
    Handle* replace(Parent parent) noexcept {
        reset();
        m_parent = parent;
        return &m_handle;
    }

    [[nodiscard]] Handle get() const noexcept { return m_handle; }
    [[nodiscard]] Parent getParent() const noexcept { return m_parent; }
    [[nodiscard]] const Handle* getAddressOf() const noexcept { return &m_handle; }

    operator Handle() const noexcept { return m_handle; }
    explicit operator bool() const noexcept { return m_handle != VK_NULL_HANDLE; }

private:
    Handle m_handle{VK_NULL_HANDLE};
    Parent m_parent{VK_NULL_HANDLE};
};

// Aliases convenientes
using UniqueVkSemaphore = UniqueVkHandle<VkSemaphore, VkDevice>;
using UniqueVkFence = UniqueVkHandle<VkFence, VkDevice>;
using UniqueVkCommandPool = UniqueVkHandle<VkCommandPool, VkDevice>;
using UniqueVkBuffer = UniqueVkHandle<VkBuffer, VkDevice>;
using UniqueVkImage = UniqueVkHandle<VkImage, VkDevice>;
using UniqueVkImageView = UniqueVkHandle<VkImageView, VkDevice>;
using UniqueVkShaderModule = UniqueVkHandle<VkShaderModule, VkDevice>;
using UniqueVkPipeline = UniqueVkHandle<VkPipeline, VkDevice>;
using UniqueVkPipelineLayout = UniqueVkHandle<VkPipelineLayout, VkDevice>;
using UniqueVkRenderPass = UniqueVkHandle<VkRenderPass, VkDevice>;
using UniqueVkFramebuffer = UniqueVkHandle<VkFramebuffer, VkDevice>;
using UniqueVkSampler = UniqueVkHandle<VkSampler, VkDevice>;
using UniqueVkDescriptorSetLayout = UniqueVkHandle<VkDescriptorSetLayout, VkDevice>;
using UniqueVkDescriptorPool = UniqueVkHandle<VkDescriptorPool, VkDevice>;
using UniqueVkSurfaceKHR = UniqueVkHandle<VkSurfaceKHR, VkInstance>;

} // namespace myve
