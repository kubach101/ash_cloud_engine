// Includes
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

// Macros
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

// Defines
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BUFFERING 3

// Structs
typedef struct
{
    const char *applicationName;
    const char *engineName;
    const char *windowTitle;

    int windowWidth, windowHeight;
    bool fullscreen;

    uint32_t apiVersion;
    VkAllocationCallbacks *allocator;

} Config;
typedef struct
{
    uint32_t imageCount;
    VkSwapchainKHR handle;
    VkImage *images;
    VkImageView *imageViews;

} Swapchain;
typedef struct
{
    GLFWwindow *handle;
    VkSurfaceKHR surface;
    Swapchain swapchain;

} Window;
typedef struct
{
    uint32_t queueFamily;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue queue;

} Context;
typedef struct
{

} Renderer;
typedef struct
{
    Config config;
    Window window;
    Context context;
    Renderer renderer;

} State;

// Helper func.
uint32_t clamp(uint32_t val, uint32_t min, uint32_t max)
{
    if (val < min)
        return min;
    else if (val > max)
        return max;
    else
        return val;
}

// Callbacks, etc.
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

// Context
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

// Window
void createSurface(State *state)
{
    PANIC_IF(glfwCreateWindowSurface(state->context.instance, state->window.handle, state->config.allocator, &state->window.surface), "Couldn't create GLFW surface\n");
}
void destroySurface(State *state)
{
    vkDestroySurfaceKHR(state->context.instance, state->window.surface, state->config.allocator);
}
void createSwapChain(State *state)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    PANIC_IF(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state->context.physicalDevice, state->window.surface, &surfaceCapabilities), "Couldn't get surface capabilities\n");

    // Surface formats:
    uint32_t surfaceFormatCount;
    PANIC_IF(vkGetPhysicalDeviceSurfaceFormatsKHR(state->context.physicalDevice, state->window.surface, &surfaceFormatCount, NULL), "Couldn't get physical device surface format count\n");
    printf("Number of surface formats detected: %i\n", surfaceFormatCount);

    VkSurfaceFormatKHR *surfaceFormats = malloc(surfaceFormatCount * sizeof(VkSurfaceFormatKHR));
    PANIC_IF(!surfaceFormats, "Couldn't allocate surface format array");
    PANIC_IF(vkGetPhysicalDeviceSurfaceFormatsKHR(state->context.physicalDevice, state->window.surface, &surfaceFormatCount, surfaceFormats), "Couldn't get physical device surface formats\n");

    uint32_t surfaceFormatIndex = UINT32_MAX;
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
    uint32_t presentModeCount;
    PANIC_IF(vkGetPhysicalDeviceSurfacePresentModesKHR(state->context.physicalDevice, state->window.surface, &presentModeCount, NULL), "Couldn't get physical device present mode count\n");
    printf("Number of present modes detected: %i\n", presentModeCount);
    VkPresentModeKHR *presentModes = malloc(presentModeCount * sizeof(VkPresentModeKHR));
    PANIC_IF(!presentModes, "Couldn't allocate present modes array\n");
    PANIC_IF(vkGetPhysicalDeviceSurfacePresentModesKHR(state->context.physicalDevice, state->window.surface, &presentModeCount, presentModes), "Couldn't get physical device present modes\n");

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
    VkExtent2D imageExtent = {
        .width = clamp(surfaceCapabilities.currentExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
        .height = clamp(surfaceCapabilities.currentExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height)};

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = state->window.surface,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &state->context.queueFamily,
        .clipped = true,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .imageArrayLayers = 1,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .oldSwapchain = state->window.swapchain.handle, // no old swapchain so it is equal to NULL
        .preTransform = surfaceCapabilities.currentTransform,
        .imageExtent = imageExtent,
        .imageFormat = surfaceFormat.format,
        .presentMode = presentMode,
        .minImageCount = clamp(BUFFERING, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount ? surfaceCapabilities.maxImageCount : UINT32_MAX),

    };

    PANIC_IF(vkCreateSwapchainKHR(state->context.device, &swapchainCreateInfo, state->config.allocator, &state->window.swapchain.handle), "Couldn't create swapchain\n");

    PANIC_IF(vkGetSwapchainImagesKHR(state->context.device, state->window.swapchain.handle, &state->window.swapchain.imageCount, NULL), "Couldn't get swapchain images count\n");
    printf("Number of swapchain images detected: %i\n", state->window.swapchain.imageCount);
    state->window.swapchain.images = malloc(state->window.swapchain.imageCount * sizeof(VkImage));
    PANIC_IF(!state->window.swapchain.images, "Couldn't allocate swapchain image array");
    PANIC_IF(vkGetSwapchainImagesKHR(state->context.device, state->window.swapchain.handle, &state->window.swapchain.imageCount, state->window.swapchain.images), "Couldn't get swapchain images\n");

    state->window.swapchain.imageViews = malloc(state->window.swapchain.imageCount * sizeof(VkImageView));
    PANIC_IF(!state->window.swapchain.imageViews, "Couldn't allocate swapchain image views array");

    for (int i = 0; i < state->window.swapchain.imageCount; i++)
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
            .image = state->window.swapchain.images[i],
            .components = componentsMapping,
            .subresourceRange = subresourceRange,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
        };
        PANIC_IF(vkCreateImageView(state->context.device, &imageViewCreateInfo, state->config.allocator, &state->window.swapchain.imageViews[i]), "Couldn't create image view\n");
    }
}
void destroySwapchain(State *state)
{
    for (int i = 0; i < state->window.swapchain.imageCount; i++)
    {
        vkDestroyImageView(state->context.device, state->window.swapchain.imageViews[i], state->config.allocator);
    }
    free(state->window.swapchain.imageViews);
    free(state->window.swapchain.images);

    vkDestroySwapchainKHR(state->context.device, state->window.swapchain.handle, state->config.allocator);
}
void createWindow(State *state)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    int windowWidth = state->config.windowWidth;
    int windowHeight = state->config.windowHeight;
    GLFWmonitor *windowMonitor = NULL;
    if (state->config.fullscreen)
    {
        windowMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(windowMonitor);
        windowWidth = mode->width;
        windowHeight = mode->height;
    }
    state->window.handle = glfwCreateWindow(windowWidth, windowHeight, state->config.windowTitle, windowMonitor, NULL);
    createSurface(state);
    createSwapChain(state);
}
void destroyWindow(State *state)
{
    destroySwapchain(state);
    destroySurface(state);
    glfwDestroyWindow(state->window.handle);
    state->window.handle = NULL;
}

// Renderer
void createRenderer(State *state)
{
}
void destroyRenderer(State *state)
{
}

// Application
void init(State *state)
{
    setupErrorHandeling();
    logInfo();

    glfwInit();
    createContext(state);
    createWindow(state);
    createRenderer(state);
}
void loop(State *state)
{
    while (!glfwWindowShouldClose(state->window.handle))
    {
        glfwPollEvents();
    }
}
void cleanup(State *state)
{
    destroyRenderer(state);
    destroyWindow(state);
    destroyContext(state);
}

// Main
int main(void)
{
    Config config = {
        .windowTitle = "VkEngine",
        .windowWidth = WINDOW_WIDTH,
        .windowHeight = WINDOW_HEIGHT,
        .fullscreen = false,
        .apiVersion = VK_API_VERSION_1_3,
        .applicationName = "VkEngine",
        .engineName = "ash_cloud",
    };

    State state = {
        .config = config,
    };

    init(&state);
    loop(&state);
    cleanup(&state);

    return 0;
}
