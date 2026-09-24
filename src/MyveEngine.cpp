#include <Myve/MyveEngine.h>
#include <Myve/Core/Exception.h>

#include <chrono>
#include <thread>

namespace myve {

MyveEngine::MyveEngine(const EngineConfig& config)
    : m_config(config) {
    // 1. Criação da janela GLFW
    WindowConfig winConfig{
        .title = m_config.windowTitle,
        .width = m_config.windowWidth,
        .height = m_config.windowHeight,
        .resizable = m_config.resizable
    };
    m_window = std::make_unique<Window>(winConfig);

    // 2. Inicialização do contexto Vulkan
    m_context = std::make_unique<VulkanContext>(*m_window, m_config.enableValidation);

    // 3. Inicialização da Swapchain
    int fbWidth = 0, fbHeight = 0;
    m_window->getFramebufferSize(&fbWidth, &fbHeight);
    m_swapchain = std::make_unique<VulkanSwapchain>(*m_context, static_cast<uint32_t>(fbWidth), static_cast<uint32_t>(fbHeight));

    // 4. Inicialização do gerenciador de memória VMA
    m_memory = std::make_unique<VulkanMemoryManager>(
        m_context->getInstance(),
        m_context->getPhysicalDevice(),
        m_context->getDevice()
    );

    // 5. Inicialização dos recursos de sincronização e command buffers
    initVulkanResources();
}

MyveEngine::~MyveEngine() {
    if (m_context) {
        m_context->waitIdle();
    }
}

void MyveEngine::initVulkanResources() {
    VkDevice device = m_context->getDevice();

    // Command Pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_context->getGraphicsQueueFamily();

    VkResult res = vkCreateCommandPool(device, &poolInfo, nullptr, m_commandPool.replace(device));
    _throwVk(res);

    // Command Buffer
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool.get();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    res = vkAllocateCommandBuffers(device, &allocInfo, &m_commandBuffer);
    _throwVk(res);

    // Semáforos e Fence
    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    res = vkCreateSemaphore(device, &semInfo, nullptr, m_imageAvailableSemaphore.replace(device));
    _throwVk(res);

    res = vkCreateSemaphore(device, &semInfo, nullptr, m_renderFinishedSemaphore.replace(device));
    _throwVk(res);

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    res = vkCreateFence(device, &fenceInfo, nullptr, m_inFlightFence.replace(device));
    _throwVk(res);
}

void MyveEngine::recreateSwapchain() {
    int width = 0, height = 0;
    m_window->getFramebufferSize(&width, &height);

    // Janela minimizada
    while (width == 0 || height == 0) {
        m_window->getFramebufferSize(&width, &height);
        m_window->pollEvents();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    m_swapchain->recreate(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    m_window->resetResizedFlag();
}

void MyveEngine::run() {
    m_running = true;
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (m_running && !m_window->shouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
        lastTime = currentTime;

        m_window->pollEvents();
        onUpdate(deltaTime);
        drawFrame();
    }

    if (m_context) {
        m_context->waitIdle();
    }
}

void MyveEngine::step() {
    if (!m_window->shouldClose()) {
        m_window->pollEvents();
        onUpdate(0.016f);
        drawFrame();
    }
}

void MyveEngine::stop() noexcept {
    m_running = false;
}

bool MyveEngine::isRunning() const noexcept {
    return m_running && !m_window->shouldClose();
}

void MyveEngine::onRender(VkCommandBuffer cmd, uint32_t imageIndex) {
    const auto& images = m_swapchain->getImages();

    // 1. Barreira: UNDEFINED -> TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier barrier1{};
    barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier1.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier1.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.image = images[imageIndex];
    barrier1.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier1.subresourceRange.baseMipLevel = 0;
    barrier1.subresourceRange.levelCount = 1;
    barrier1.subresourceRange.baseArrayLayer = 0;
    barrier1.subresourceRange.layerCount = 1;
    barrier1.srcAccessMask = 0;
    barrier1.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier1
    );

    // 2. Limpar a tela (Cor de fundo: Azul acinzentado escuro)
    VkClearColorValue clearColor = {{0.08f, 0.09f, 0.12f, 1.0f}};
    VkImageSubresourceRange range{};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    vkCmdClearColorImage(
        cmd,
        images[imageIndex],
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        &clearColor,
        1,
        &range
    );

    // 3. Barreira: TRANSFER_DST_OPTIMAL -> PRESENT_SRC_KHR
    VkImageMemoryBarrier barrier2 = barrier1;
    barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier2.dstAccessMask = 0;

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier2
    );
}

void MyveEngine::drawFrame() {
    VkDevice device = m_context->getDevice();
    VkFence fence = m_inFlightFence.get();

    vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult acquireRes = m_swapchain->acquireNextImage(
        m_imageAvailableSemaphore.get(),
        VK_NULL_HANDLE,
        &imageIndex
    );

    if (acquireRes == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    } else if (acquireRes != VK_SUCCESS && acquireRes != VK_SUBOPTIMAL_KHR) {
        _throwVk(acquireRes);
    }

    vkResetFences(device, 1, &fence);

    // Gravação dos comandos
    vkResetCommandBuffer(m_commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

    onRender(m_commandBuffer, imageIndex);

    vkEndCommandBuffer(m_commandBuffer);

    // Submissão na fila
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore.get()};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_TRANSFER_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffer;

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore.get()};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkResult submitRes = vkQueueSubmit(m_context->getGraphicsQueue(), 1, &submitInfo, fence);
    _throwVk(submitRes);

    // Apresentação na tela
    VkSwapchainKHR swapchains[] = {m_swapchain->getSwapchain()};
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    VkResult presentRes = vkQueuePresentKHR(m_context->getPresentQueue(), &presentInfo);

    if (presentRes == VK_ERROR_OUT_OF_DATE_KHR || presentRes == VK_SUBOPTIMAL_KHR || m_window->wasResized()) {
        recreateSwapchain();
    } else if (presentRes != VK_SUCCESS) {
        _throwVk(presentRes);
    }
}

} // namespace myve
