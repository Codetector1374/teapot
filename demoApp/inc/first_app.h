#pragma once

#include "tp_device.h"
#include "tp_pipeline.h"
#include "tp_swap_chain.h"
#include "tp_window.h"
#include "tp_model.h"

// std
#include <memory>
#include <vector>

namespace tpApp {

using namespace teapot;

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
  void loadModel();
  void recreateSwapChain();
  void recordCommandBuffer(uint32_t imageIndex);
  void freeCommandBuffers();

  teapot::TpWindow tpWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
  teapot::TpDevice tpDevice{tpWindow};
  std::unique_ptr<teapot::TpSwapChain> tpSwapChain;
  std::unique_ptr<teapot::TpPipeline> tpPipeline;
  VkPipelineLayout pipelineLayout{};
  std::vector<VkCommandBuffer> commandBuffers;
  std::unique_ptr<TpModel> tpModel;
};
}  // namespace teapot