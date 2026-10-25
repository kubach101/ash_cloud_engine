#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#define PANIC_IF(ERROR, FORMAT, ...)                                              \
    do                                                                            \
    {                                                                             \
        int panic_err_ = (ERROR);                                                 \
        if (panic_err_)                                                           \
        {                                                                         \
            fprintf(stderr,                                                       \
                    "\033[1;31m%s -> %s -> %i -> Error(%i): " FORMAT "\033[0m\n", \
                    __FILE__, __func__, __LINE__, panic_err_, ##__VA_ARGS__);     \
            exit(1);                                                              \
        }                                                                         \
    } while (0)

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BUFFERING 3

void glfwErrorCallback(int error, const char *description)
{
    PANIC_IF(error, "GLFW Error: %s\n", description);
}

void exitCallback()
{
    glfwTerminate();
}

void setupErrorHandeling()
{
    glfwSetErrorCallback(glfwErrorCallback);
    atexit(exitCallback);
}

typedef struct
{
    const char *applicationName;
    const char *engineName;
    const char *windowTitle;

    int windowWidth, windowHeight;
    bool fullscreen;

    // GLFW

    GLFWmonitor *windowMonitor;
    GLFWwindow *window;

    // Vulkan
    uint32_t apiVersion;
    uint32_t queueFamily;
    uint32_t swapchainImageCount;

    VkAllocationCallbacks *allocator;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    VkDevice device;
    VkQueue queue;

    VkSwapchainKHR swapchain;
    VkImage *swapchainImages;
    VkImageView *swapchainImageViews;

} State;

void createWindow(State *state)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    if (state->fullscreen)
    {
        state->windowMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(state->windowMonitor);
        state->windowWidth = mode->width;
        state->windowHeight = mode->height;
    }
    state->window = glfwCreateWindow(state->windowWidth, state->windowHeight, state->windowTitle, state->windowMonitor, NULL);
}

void createInstance(State *state)
{
    uint32_t extensionCount;
    const char **requiredExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    VkApplicationInfo applicationInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = state->apiVersion,
        .pApplicationName = state->applicationName,
        .pEngineName = state->engineName,
    };
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &applicationInfo,
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = requiredExtensions,
    };
    PANIC_IF(vkCreateInstance(&createInfo, state->allocator, &state->instance), "Couldn't create VkInstance\n");
}
void logInfo()
{
    uint32_t instanceApiVersion;
    PANIC_IF(vkEnumerateInstanceVersion(&instanceApiVersion), "Couldn't enumerate VkInstance\n");
    uint32_t apiVersionVariant = VK_API_VERSION_VARIANT(instanceApiVersion);
    uint32_t apiVersionMajor = VK_API_VERSION_MAJOR(instanceApiVersion);
    uint32_t apiVersionMinor = VK_API_VERSION_MINOR(instanceApiVersion);
    uint32_t apiVersionPatch = VK_API_VERSION_PATCH(instanceApiVersion);
    printf("Vulkan API: %i.%i.%i.%i\n", apiVersionVariant, apiVersionMajor, apiVersionMinor, apiVersionPatch);
    printf("GLFW: %s\n", glfwGetVersionString());
}
void selectPhysicalDevice(State *state)
{
    uint32_t physicalDeviceCount;
    PANIC_IF(vkEnumeratePhysicalDevices(state->instance, &physicalDeviceCount, NULL), "Couldn't enumerate VkPhysicalDevice -step1\n");
    printf("Vulkan supported physical devices detected: %i\n", physicalDeviceCount);
    PANIC_IF(physicalDeviceCount == 0, "Couldn't find vulkan supported physical devices\n");
    PANIC_IF(vkEnumeratePhysicalDevices((state->instance), &(uint32_t){1}, &state->physicalDevice) < 0, "Couldn't enumerate VkPhysicalDevice -step2\n"); // Windows will order the devices
}
void createSurface(State *state)
{
    PANIC_IF(glfwCreateWindowSurface(state->instance, state->window, state->allocator, &state->surface), "Couldn't create GLFW surface\n");
}
void selectQueueFamily(State *state)
{
    state->queueFamily = UINT32_MAX;
    uint32_t queueFamilyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(state->physicalDevice, &queueFamilyCount, NULL);
    VkQueueFamilyProperties *queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties));
    PANIC_IF(queueFamilies == NULL, "Couldn't allocate queueFamilies\n");
    vkGetPhysicalDeviceQueueFamilyProperties(state->physicalDevice, &queueFamilyCount, queueFamilies);

    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount; queueFamilyIndex++)
    {
        VkQueueFamilyProperties properties = queueFamilies[queueFamilyIndex];
        if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT && glfwGetPhysicalDevicePresentationSupport(state->instance, state->physicalDevice, queueFamilyIndex))
        {
            state->queueFamily = queueFamilyIndex;
            break;
        }
    }
    PANIC_IF(state->queueFamily == UINT32_MAX, "Couldn't find queue families\n");
    free(queueFamilies);
}
void createDevice(State *state)
{
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = state->queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &(float){1.0}};
    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = &queueCreateInfo,
        .queueCreateInfoCount = 1,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = &(const char *){VK_KHR_SWAPCHAIN_EXTENSION_NAME},
    };
    PANIC_IF(vkCreateDevice(state->physicalDevice, &deviceCreateInfo, state->allocator, &state->device), "Couldn't create devices and queues\n");
}
void getQueue(State *state)
{
    vkGetDeviceQueue(state->device, state->queueFamily, 0, &state->queue);
}
uint32_t clamp(uint32_t val, uint32_t min, uint32_t max)
{
    if (val < min)
        return min;
    else if (val > max)
        return max;
    else
        return val;
}
void createSwapChain(State *state)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    PANIC_IF(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state->physicalDevice, state->surface, &surfaceCapabilities), "Couldn't get surface capabilities\n");

    // Surface formats:
    uint32_t surfaceFormatCount;
    PANIC_IF(vkGetPhysicalDeviceSurfaceFormatsKHR(state->physicalDevice, state->surface, &surfaceFormatCount, NULL), "Couldn't get physical device surface format count\n");
    printf("Number of surface formats detected: %i\n", surfaceFormatCount);

    VkSurfaceFormatKHR *surfaceFormats = malloc(surfaceFormatCount * sizeof(VkSurfaceFormatKHR));
    PANIC_IF(!surfaceFormats, "Couldn't allocate surface format array");
    PANIC_IF(vkGetPhysicalDeviceSurfaceFormatsKHR(state->physicalDevice, state->surface, &surfaceFormatCount, surfaceFormats), "Couldn't get physical device surface formats\n");

    int32_t surfaceFormatIndex = UINT32_MAX;
    for (int i = 0; i < surfaceFormatCount; i++)
    {
        if (surfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && surfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            surfaceFormatIndex = i;
            break;
        }
    }

    PANIC_IF(surfaceFormatIndex == UINT32_MAX, "Couldn't fint suitable surface format\n");
    VkSurfaceFormatKHR surfaceFormat = surfaceFormats[surfaceFormatIndex];
    printf("Selected surface format: SRGB nonlinear\n");
    free(surfaceFormats);

    // Present modes:
    int32_t presentModeCount;
    PANIC_IF(vkGetPhysicalDeviceSurfacePresentModesKHR(state->physicalDevice, state->surface, &presentModeCount, NULL), "Couldn't get physical device present mode count\n");
    printf("Number of present modes detected: %i\n", presentModeCount);
    VkPresentModeKHR *presentModes = malloc(presentModeCount * sizeof(VkPresentModeKHR));
    PANIC_IF(!presentModes, "Couldn't allocate present modes array\n");
    PANIC_IF(vkGetPhysicalDeviceSurfacePresentModesKHR(state->physicalDevice, state->surface, &presentModeCount, presentModes), "Couldn't get physical device present modes\n");

    uint32_t presentModeIndex = UINT32_MAX;
    for (int i = 0; i < presentModeCount; i++)
    {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentModeIndex = i;

            break;
        }
    }
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (presentModeIndex != UINT32_MAX)
    {
        presentMode = presentModes[presentModeIndex];
        printf("Selected present mode: MAILBOX\n");
    }
    else
    {
        printf("Selected present mode: FIFO\n");
    }
    free(presentModes);

    // Swapchain:
    VkSwapchainKHR swapchain;
    VkExtent2D imageExtent = {
        .width = clamp(surfaceCapabilities.currentExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
        .height = clamp(surfaceCapabilities.currentExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height)};

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = state->surface,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &state->queueFamily,
        .clipped = true,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .imageArrayLayers = 1,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .oldSwapchain = state->swapchain,
        .preTransform = surfaceCapabilities.currentTransform,
        .imageExtent = imageExtent,
        .imageFormat = surfaceFormat.format,
        .presentMode = presentMode,
        .minImageCount = clamp(BUFFERING, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount ? surfaceCapabilities.maxImageCount : UINT32_MAX),

    };

    PANIC_IF(vkCreateSwapchainKHR(state->device, &swapchainCreateInfo, state->allocator, &swapchain), "Couldn't create swapchain\n");

    if (!state->swapchainImages)
    {
        for (int i = 0; i < state->swapchainImageCount; i++)
        {
            vkDestroyImageView(state->device, state->swapchainImageViews[i], state->allocator);
        }
        free(state->swapchainImageViews);
        free(state->swapchainImages);
    }

    vkDestroySwapchainKHR(state->device, state->swapchain, state->allocator);
    state->swapchain = swapchain;

    PANIC_IF(vkGetSwapchainImagesKHR(state->device, state->swapchain, &state->swapchainImageCount, NULL), "Couldn't get swapchain images count\n");
    printf("Number of swapchain images detected: %i\n", state->swapchainImageCount);
    state->swapchainImages = malloc(state->swapchainImageCount * sizeof(VkImage));
    PANIC_IF(!state->swapchainImages, "Couldn't allocate swapchain image array");
    PANIC_IF(vkGetSwapchainImagesKHR(state->device, state->swapchain, &state->swapchainImageCount, state->swapchainImages), "Couldn't get swapchain images\n");

    state->swapchainImageViews = malloc(state->swapchainImageCount * sizeof(VkImageView));
    PANIC_IF(!state->swapchainImageViews, "Couldn't allocate swapchain image views array");

    for (int i = 0; i < state->swapchainImageCount; i++)
    {
        VkComponentMapping componentsMapping = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY};
        VkImageSubresourceRange subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .layerCount = 1,
            .levelCount = 1};
        VkImageViewCreateInfo imageViewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .format = surfaceFormat.format,
            .image = state->swapchainImages[i],
            .components = componentsMapping,
            .subresourceRange = subresourceRange,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
        };
        PANIC_IF(vkCreateImageView(state->device, &imageViewCreateInfo, state->allocator, &state->swapchainImageViews[i]), "Couldn't create image view\n");
    }
}
void init(State *state)
{
    setupErrorHandeling();
    logInfo();

    createWindow(state);
    createInstance(state);

    selectPhysicalDevice(state);
    createSurface(state);
    selectQueueFamily(state);
    createDevice(state);
    getQueue(state);
    createSwapChain(state);
}
void loop(State *state)
{
    while (!glfwWindowShouldClose(state->window))
    {
        glfwPollEvents();
    }
}
void cleanup(State *state)
{
    for (int i = 0; i < state->swapchainImageCount; i++)
    {
        vkDestroyImageView(state->device, state->swapchainImageViews[i], state->allocator);
    }
    free(state->swapchainImageViews);
    free(state->swapchainImages);

    vkDestroySwapchainKHR(state->device, state->swapchain, state->allocator);
    vkDestroyDevice(state->device, state->allocator);
    vkDestroySurfaceKHR(state->instance, state->surface, state->allocator);
    vkDestroyInstance(state->instance, state->allocator);

    glfwDestroyWindow(state->window);
    state->windowMonitor = NULL;
    state->window = NULL;
}
int main(void)
{
    State state = {
        .windowTitle = "VkEngine",
        .windowWidth = WINDOW_WIDTH,
        .windowHeight = WINDOW_HEIGHT,
        .fullscreen = false,
        .apiVersion = VK_API_VERSION_1_3,
        .applicationName = "VkEngine",
        .engineName = "ash_cloud",
    };

    init(&state);
    loop(&state);
    cleanup(&state);

    return 0;
}
