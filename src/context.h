#pragma once

void createInstance(State *state)
{
    uint32_t extensionCount;
    const char **requiredExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = state->config.apiVersion,
        .pApplicationName = state->config.applicationName,
        .pEngineName = state->config.engineName,
    };
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &applicationInfo,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = requiredExtensions,
    };
    PANIC_IF(vkCreateInstance(&createInfo, state->config.allocator, &state->context.instance), "Couldn't create VkInstance\n");
}
void selectPhysicalDevice(State *state)
{
    uint32_t physicalDeviceCount;
    PANIC_IF(vkEnumeratePhysicalDevices(state->context.instance, &physicalDeviceCount, NULL), "Couldn't enumerate VkPhysicalDevice -step1\n");
    printf("Vulkan supported physical devices detected: %i\n", physicalDeviceCount);
    PANIC_IF(physicalDeviceCount == 0, "Couldn't find vulkan supported physical devices\n");
    PANIC_IF(vkEnumeratePhysicalDevices((state->context.instance), &(uint32_t){1}, &state->context.physicalDevice) < 0, "Couldn't enumerate VkPhysicalDevice -step2\n"); // Windows will order the devices
}
void selectQueueFamily(State *state)
{
    state->context.queueFamily = UINT32_MAX;
    uint32_t queueFamilyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(state->context.physicalDevice, &queueFamilyCount, NULL);
    VkQueueFamilyProperties *queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    PANIC_IF(queueFamilies == NULL, "Couldn't allocate queueFamilies\n");
    vkGetPhysicalDeviceQueueFamilyProperties(state->context.physicalDevice, &queueFamilyCount, queueFamilies);

    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; queueFamilyIndex++)
    {
        VkQueueFamilyProperties properties = queueFamilies[queueFamilyIndex];
        if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT && glfwGetPhysicalDevicePresentationSupport(state->context.instance, state->context.physicalDevice, queueFamilyIndex))
        {
            state->context.queueFamily = queueFamilyIndex;
            break;
        }
    }
    PANIC_IF(state->context.queueFamily == UINT32_MAX, "Couldn't find queue families\n");
    free(queueFamilies);
}
void createDevice(State *state)
{
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = state->context.queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &(float){1.0}};
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = &queueCreateInfo,
        .queueCreateInfoCount = 1,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = &(const char *){VK_KHR_SWAPCHAIN_EXTENSION_NAME},
    };
    PANIC_IF(vkCreateDevice(state->context.physicalDevice, &deviceCreateInfo, state->config.allocator, &state->context.device), "Couldn't create devices and queues\n");
}
void getQueue(State *state)
{
    vkGetDeviceQueue(state->context.device, state->context.queueFamily, 0, &state->context.queue);
}
void createContext(State *state)
{
    createInstance(state);

    selectPhysicalDevice(state);

    selectQueueFamily(state);
    createDevice(state);
    getQueue(state);
}
void destroyContext(State *state)
{
    vkDestroyDevice(state->context.device, state->config.allocator);
    vkDestroyInstance(state->context.instance, state->config.allocator);
}
