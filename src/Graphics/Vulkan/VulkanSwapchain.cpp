#include <Myve/Graphics/Vulkan/VulkanSwapchain.h>
#include <Myve/Core/Exception.h>

namespace myve {

VulkanSwapchain::VulkanSwapchain(const VulkanContext& context, uint32_t width, uint32_t height)
    : m_context(&context) {
    init(width, height);
}

VulkanSwapchain::~VulkanSwapchain() {
    cleanup();
}

void VulkanSwapchain::init(uint32_t width, uint32_t height) {
    if (!m_context) {
        _throwMsg("VulkanContext nulo ao inicializar Swapchain.");
    }

    vkb::SwapchainBuilder swapchainBuilder{m_context->getVkbDevice()};
    auto swapRet = swapchainBuilder
        .set_desired_extent(width, height)
        .set_desired_format({VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        .build();

    if (!swapRet) {
        _throwMsg("Falha ao criar Swapchain via vk-bootstrap: " + swapRet.error().message());
    }

    m_vkbSwapchain = swapRet.value();
    m_swapchain = m_vkbSwapchain.swapchain;
    m_extent = m_vkbSwapchain.extent;
    m_imageFormat = m_vkbSwapchain.image_format;

    auto imagesRet = m_vkbSwapchain.get_images();
    if (!imagesRet) {
        _throwMsg("Falha ao obter imagens da swapchain: " + imagesRet.error().message());
    }
    m_images = imagesRet.value();

    auto viewsRet = m_vkbSwapchain.get_image_views();
    if (!viewsRet) {
        _throwMsg("Falha ao criar image views da swapchain: " + viewsRet.error().message());
    }
    m_imageViews = viewsRet.value();
}

void VulkanSwapchain::cleanup() {
    if (m_swapchain != VK_NULL_HANDLE) {
        if (!m_imageViews.empty()) {
            m_vkbSwapchain.destroy_image_views(m_imageViews);
            m_imageViews.clear();
        }
        vkb::destroy_swapchain(m_vkbSwapchain);
        m_swapchain = VK_NULL_HANDLE;
    }
    m_images.clear();
}

void VulkanSwapchain::recreate(uint32_t width, uint32_t height) {
    if (m_context) {
        m_context->waitIdle();
    }
    cleanup();
    init(width, height);
}

VkResult VulkanSwapchain::acquireNextImage(
    VkSemaphore imageAvailableSemaphore,
    VkFence fence,
    uint32_t* imageIndex,
    uint64_t timeoutNs
) const noexcept {
    if (!m_context || m_swapchain == VK_NULL_HANDLE) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    return vkAcquireNextImageKHR(
        m_context->getDevice(),
        m_swapchain,
        timeoutNs,
        imageAvailableSemaphore,
        fence,
        imageIndex
    );
}

VulkanSwapchain::VulkanSwapchain(VulkanSwapchain&& other) noexcept
    : m_context(other.m_context),
      m_vkbSwapchain(other.m_vkbSwapchain),
      m_swapchain(other.m_swapchain),
      m_extent(other.m_extent),
      m_imageFormat(other.m_imageFormat),
      m_images(std::move(other.m_images)),
      m_imageViews(std::move(other.m_imageViews)) {
    other.m_swapchain = VK_NULL_HANDLE;
    other.m_images.clear();
    other.m_imageViews.clear();
}

VulkanSwapchain& VulkanSwapchain::operator=(VulkanSwapchain&& other) noexcept {
    if (this != &other) {
        cleanup();
        m_context = other.m_context;
        m_vkbSwapchain = other.m_vkbSwapchain;
        m_swapchain = other.m_swapchain;
        m_extent = other.m_extent;
        m_imageFormat = other.m_imageFormat;
        m_images = std::move(other.m_images);
        m_imageViews = std::move(other.m_imageViews);

        other.m_swapchain = VK_NULL_HANDLE;
        other.m_images.clear();
        other.m_imageViews.clear();
    }
    return *this;
}

} // namespace myve
