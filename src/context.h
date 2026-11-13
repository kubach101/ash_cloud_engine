#pragma once

#include "headers.h"
#include "utils.h"
#include "structs.h"


bool isLayerAvailable(const char *layerName)
{
    uint32_t layerCount;
    PANIC_IF(vkEnumerateInstanceLayerProperties(&layerCount, NULL), "Couldn't get instance layers count\n");
    VkLayerProperties *layers = malloc(layerCount * sizeof(VkLayerProperties));
    PANIC_IF(layers == NULL, "Couldn't allocate layers\n");
    PANIC_IF(vkEnumerateInstanceLayerProperties(&layerCount, layers), "Couldn't enumerate instance layers\n");

    bool found = false;
    for (uint32_t layerIndex = 0; layerIndex < layerCount; layerIndex++)
    {
        if (strcmp(layers[layerIndex].layerName, layerName) == 0)
        {
            found = true;
            break;
        }
    }
    free(layers);
    return found;
}
void createInstance(State *state)
{
    const char *layers[] = {"VK_LAYER_KHRONOS_validation"};
    uint32_t layerCount = 0;
#ifndef NDEBUG
    if (isLayerAvailable(layers[0]))
        layerCount = 1;
    else
        fprintf(stderr, "Validation layer %s not available, install the Vulkan SDK\n", layers[0]);
#endif

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
        .enabledLayerCount = layerCount,
        .ppEnabledLayerNames = layers,
    };
    PANIC_IF(vkCreateInstance(&createInfo, state->config.allocator, &state->context.instance), "Couldn't create VkInstance\n");
}
void selectPhysicalDevice(State *state)
{
    uint32_t physicalDeviceCount;
    PANIC_IF(vkEnumeratePhysicalDevices(state->context.instance, &physicalDeviceCount, NULL), "Couldn't get physical devices count\n");
    printf("Vulkan supported physical devices detected: %i\n", physicalDeviceCount);
    PANIC_IF(physicalDeviceCount == 0, "Couldn't find vulkan supported physical devices\n");
    VkResult result = vkEnumeratePhysicalDevices((state->context.instance), &(uint32_t){1}, &state->context.physicalDevice); // Windows will order the devices
    if (result != VK_INCOMPLETE)
        PANIC_IF(result, "Couldn't enumerate physical devices\n");
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
