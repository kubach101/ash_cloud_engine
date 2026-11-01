#pragma once

#define BUFFERING 3
void createSurface(State *state)
{
    PANIC_IF(glfwCreateWindowSurface(state->context.instance, state->window.handle, state->config.allocator, &state->window.surface), "Couldn't create GLFW surface\n");
}
void destroySurface(State *state)
{
    vkDestroySurfaceKHR(state->context.instance, state->window.surface, state->config.allocator);
}

VkSurfaceCapabilitiesKHR getSurfaceCapabilities(const State *state)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    PANIC_IF(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state->context.physicalDevice, state->window.surface, &surfaceCapabilities), "Couldn't get surface capabilities\n");
    return surfaceCapabilities;
}
VkSurfaceFormatKHR selectSurfaceFormat(State *state)
{
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
    return surfaceFormat;
}
VkPresentModeKHR getPresentMode(State *state)
{

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
    return presentMode;
}

void getSwapchainImages(State *state)
{
    PANIC_IF(vkGetSwapchainImagesKHR(state->context.device, state->window.swapchain.handle, &state->window.swapchain.imageCount, NULL), "Couldn't get swapchain images count\n");
    printf("Number of swapchain images detected: %i\n", state->window.swapchain.imageCount);
    state->window.swapchain.images = malloc(state->window.swapchain.imageCount * sizeof(VkImage));
    PANIC_IF(!state->window.swapchain.images, "Couldn't allocate swapchain image array");
    PANIC_IF(vkGetSwapchainImagesKHR(state->context.device, state->window.swapchain.handle, &state->window.swapchain.imageCount, state->window.swapchain.images), "Couldn't get swapchain images\n");
}
void createSwapchainImageViews(State *state)
{
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
            .format = state->window.swapchain.format,
            .image = state->window.swapchain.images[i],
            .components = state->config.swapchainComponentsMapping,
            .subresourceRange = subresourceRange,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
        };
        PANIC_IF(vkCreateImageView(state->context.device, &imageViewCreateInfo, state->config.allocator, &state->window.swapchain.imageViews[i]), "Couldn't create image view\n");
    }
}
void createSwapchain(State *state)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities = getSurfaceCapabilities(state);
    VkSurfaceFormatKHR surfaceFormat = selectSurfaceFormat(state);
    VkPresentModeKHR presentMode = getPresentMode(state);

    state->window.swapchain.format = surfaceFormat.format;
    state->window.swapchain.imageExtend = (VkExtent2D){
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
        .imageExtent = state->window.swapchain.imageExtend,
        .imageFormat = state->window.swapchain.format,
        .presentMode = presentMode,
        .minImageCount = clamp(state->config.swapchainBuffering, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount ? surfaceCapabilities.maxImageCount : UINT32_MAX),

    };

    PANIC_IF(vkCreateSwapchainKHR(state->context.device, &swapchainCreateInfo, state->config.allocator, &state->window.swapchain.handle), "Couldn't create swapchain\n");
    getSwapchainImages(state);
    createSwapchainImageViews(state);
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
    createSwapchain(state);
}
void destroyWindow(State *state)
{
    destroySwapchain(state);
    destroySurface(state);
    glfwDestroyWindow(state->window.handle);
    state->window.handle = NULL;
}
