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

} Config;
typedef struct {
  uint32_t imageCount;
  VkSwapchainKHR handle;
  VkImage *images;
  VkImageView *imageViews;
  VkFormat format;
  VkColorSpaceKHR colorSpace;
  VkExtent2D imageExtend;

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
} Renderer;
typedef struct {
  Config config;
  Window window;
  Context context;
  Renderer renderer;

} State;

typedef struct {
  float v[3];
} vec3;
typedef struct {
  int type;
  vec3 pos; // todo: full mat4 model matrix after getting cglm
  float scale;
} Body;

enum swapchainBuffering {
  SWAPCHAIN_DOUBLE_BUFFERING = 2,
  SWAPCHAIN_TRIPLE_BUFFERING = 3,
};

enum bodyType {
  SPHERE = 0,
  CUBE = 1,
};