#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cstdlib>

int main() {
    // 1. Inicializa GLFW
    if (!glfwInit()) {
        std::cerr << "Falha ao inicializar o GLFW.\n";
        return EXIT_FAILURE;
    }

    if (!glfwVulkanSupported()) {
        std::cerr << "GLFW: Vulkan nao e suportado no sistema.\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Informa ao GLFW para nao criar um contexto OpenGL
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    constexpr uint32_t WIDTH = 800;
    constexpr uint32_t HEIGHT = 600;
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Bootstrap", nullptr, nullptr);

    if (!window) {
        std::cerr << "Falha ao criar janela GLFW.\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // 2. Coleta extensoes necessarias pelo GLFW para Vulkan
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    std::cout << "Extensoes exigidas pelo GLFW:\n";
    for (const auto& ext : extensions) {
        std::cout << "  - " << ext << '\n';
    }

    // 3. Configuracao basica da VkInstance
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Bootstrap";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;

    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        std::cerr << "Falha ao criar VkInstance. Codigo: " << result << '\n';
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    std::cout << "\nVkInstance criada com sucesso!\n";

    // 4. Loop simples para manter a janela aberta ate fechar
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // 5. Cleanup
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
