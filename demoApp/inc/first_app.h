#pragma once

#include "tp_device.h"
#include "tp_pipeline.h"
#include "tp_swap_chain.h"
#include "tp_window.h"

// std
#include <memory>
#include <vector>

namespace tpApp {

class FirstApp {
 public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  FirstApp();
  ~FirstApp();

  FirstApp(const FirstApp &) = delete;
  FirstApp &operator=(const FirstApp &) = delete;

  void run();

 private:
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void drawFrame();

  teapot::TpWindow lveWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
  teapot::TpDevice lveDevice{lveWindow};
  teapot::TpSwapChain lveSwapChain{lveDevice, lveWindow.getExtent()};
  std::unique_ptr<teapot::TpPipeline> lvePipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkCommandBuffer> commandBuffers;
};
}  // namespace teapot