#include <Myve/Graphics/Vulkan/VulkanContext.h>
#include <Myve/Core/Exception.h>

namespace myve {

VulkanContext::VulkanContext(const Window& window, bool enableValidation) {
    // 1. Instance com validação opcional (habilitada por padrão em desenvolvimento)
    vkb::InstanceBuilder instBuilder;
    auto instRet = instBuilder.set_app_name("Myve")
                       .request_validation_layers(enableValidation)
                       .use_default_debug_messenger()
                       .require_api_version(1, 3, 0)
                       .build();

    if (!instRet) {
        _throwMsg("Falha ao criar Vulkan Instance via vk-bootstrap: " + instRet.error().message());
    }
    m_vkbInstance = instRet.value();
    m_instance = m_vkbInstance.instance;

    // 2. Surface via Window
    VkResult surfRes = window.createSurface(m_instance, &m_surface);
    _throwVk(surfRes);

    // 3. Seleção da Physical Device (GPU discreta com suporte à surface e Vulkan 1.3)
    vkb::PhysicalDeviceSelector physSelector{m_vkbInstance};
    auto physRet = physSelector.set_surface(m_surface)
                       .set_minimum_version(1, 3)
                       .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
                       .select();

    if (!physRet) {
        _throwMsg("Falha ao selecionar Physical Device compatível: " + physRet.error().message());
    }
    m_vkbPhysicalDevice = physRet.value();
    m_physicalDevice = m_vkbPhysicalDevice.physical_device;

    // 4. Logical Device e Filas
    vkb::DeviceBuilder deviceBuilder{m_vkbPhysicalDevice};
    auto devRet = deviceBuilder.build();
    if (!devRet) {
        _throwMsg("Falha ao criar Logical Device: " + devRet.error().message());
    }
    m_vkbDevice = devRet.value();
    m_device = m_vkbDevice.device;

    // Filas
    auto gQueue = m_vkbDevice.get_queue(vkb::QueueType::graphics);
    if (!gQueue) {
        _throwMsg("Falha ao obter fila gráfica: " + gQueue.error().message());
    }
    m_graphicsQueue = gQueue.value();
    m_graphicsQueueFamily = m_vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    auto pQueue = m_vkbDevice.get_queue(vkb::QueueType::present);
    if (!pQueue) {
        _throwMsg("Falha ao obter fila de apresentação: " + pQueue.error().message());
    }
    m_presentQueue = pQueue.value();
    m_presentQueueFamily = m_vkbDevice.get_queue_index(vkb::QueueType::present).value();
}

VulkanContext::~VulkanContext() {
    cleanup();
}

void VulkanContext::cleanup() {
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
        vkb::destroy_device(m_vkbDevice);
        m_device = VK_NULL_HANDLE;
    }

    if (m_surface != VK_NULL_HANDLE && m_instance != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }

    if (m_instance != VK_NULL_HANDLE) {
        vkb::destroy_instance(m_vkbInstance);
        m_instance = VK_NULL_HANDLE;
    }
}

VulkanContext::VulkanContext(VulkanContext&& other) noexcept
    : m_vkbInstance(other.m_vkbInstance),
      m_vkbPhysicalDevice(other.m_vkbPhysicalDevice),
      m_vkbDevice(other.m_vkbDevice),
      m_instance(other.m_instance),
      m_surface(other.m_surface),
      m_physicalDevice(other.m_physicalDevice),
      m_device(other.m_device),
      m_graphicsQueue(other.m_graphicsQueue),
      m_graphicsQueueFamily(other.m_graphicsQueueFamily),
      m_presentQueue(other.m_presentQueue),
      m_presentQueueFamily(other.m_presentQueueFamily) {
    other.m_instance = VK_NULL_HANDLE;
    other.m_surface = VK_NULL_HANDLE;
    other.m_device = VK_NULL_HANDLE;
}

VulkanContext& VulkanContext::operator=(VulkanContext&& other) noexcept {
    if (this != &other) {
        cleanup();
        m_vkbInstance = other.m_vkbInstance;
        m_vkbPhysicalDevice = other.m_vkbPhysicalDevice;
        m_vkbDevice = other.m_vkbDevice;
        m_instance = other.m_instance;
        m_surface = other.m_surface;
        m_physicalDevice = other.m_physicalDevice;
        m_device = other.m_device;
        m_graphicsQueue = other.m_graphicsQueue;
        m_graphicsQueueFamily = other.m_graphicsQueueFamily;
        m_presentQueue = other.m_presentQueue;
        m_presentQueueFamily = other.m_presentQueueFamily;

        other.m_instance = VK_NULL_HANDLE;
        other.m_surface = VK_NULL_HANDLE;
        other.m_device = VK_NULL_HANDLE;
    }
    return *this;
}

void VulkanContext::waitIdle() const {
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
    }
}

} // namespace myve
