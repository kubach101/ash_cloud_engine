#include "toolkit.h"

int main(void) {
  Config config = {.windowTitle = "VkEngine",
                   .windowWidth = 800,
                   .windowHeight = 600,
                   .fullscreen = false,
                   .apiVersion = VK_API_VERSION_1_3,
                   .applicationName = "VkEngine",
                   .engineName = "ash_cloud",
                   .swapchainComponentsMapping = (VkComponentMapping){},
                   .swapchainBuffering = SWAPCHAIN_TRIPLE_BUFFERING,
                   .backgroundColor = {{0.03f, 0.03f, 0.03f, 1.0f}}};

  State state = {
      .config = config,
  };

  init(&state);
  loop(&state);
  cleanup(&state);

  return EXIT_SUCCESS;
}
