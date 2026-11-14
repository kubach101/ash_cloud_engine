#pragma once

#include "headers.h"

typedef struct {
  const char *applicationName;
  const char *engineName;
  const char *windowTitle;

  int windowWidth, windowHeight;
  bool fullscreen;

  uint32_t apiVersion;
  VkAllocationCallbacks *allocator;
  VkComponentMapping swapchainComponentsMapping;
  uint32_t swapchainBuffering;

  VkClearValue backgroundColor;

} Config;
typedef struct {
  uint32_t imageCount;
  VkSwapchainKHR handle;
  VkImage *images;
  VkImageView *imageViews;
  VkFormat format;
  VkColorSpaceKHR colorSpace;
  VkExtent2D imageExtend;
  uint32_t imageAcquiredIndex;

} Swapchain;
typedef struct {
  GLFWwindow *handle;
  VkSurfaceKHR surface;
  Swapchain swapchain;

} Window;
typedef struct {
  uint32_t queueFamily;
  VkInstance instance;
  VkPhysicalDevice physicalDevice;
  VkDevice device;
  VkQueue queue;

} Context;

typedef struct {
  VkPipeline graphicsPipeline;
  VkPipelineLayout graphicsPipelineLayout;
  VkRenderPass renderPass;
  VkFramebuffer *framebuffers;
  VkCommandPool commandPool;
  VkCommandBuffer *commandBuffers;
  VkSemaphore imageAcquiredSemaphore;
  VkSemaphore *renderFinishedSemaphores; // one per swapchain image
  VkFence inFlightFence;

  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  VkDescriptorSet descriptorSet;
  VkBuffer sphereBuffer;
  VkDeviceMemory sphereMemory;
  void *sphereMapped;
} Renderer;
typedef struct {
  Config config;
  Window window;
  Context context;
  Renderer renderer;

} State;

typedef struct {
  float player[3];
  float fov;            // offset 0
  float resolution[2];  // offset 16
  float boundingRadius; // offset 24
  uint32_t sphereCount; // offset 28
} PushConstants;        // 32 bytes

enum swapchainBuffering {
  SWAPCHAIN_DOUBLE_BUFFERING = 2,
  SWAPCHAIN_TRIPLE_BUFFERING = 3,
};

enum bodyType {
  SPHERE = 0,
  CUBE = 1,
};