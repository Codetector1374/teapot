//
// Created by user on 6/30/2021.
//

#ifndef TEAPOT_TP_RENDERER_H
#define TEAPOT_TP_RENDERER_H

#include "tp_device.h"
#include "tp_swap_chain.h"
#include "tp_window.h"

#include <memory>
#include <vector>
#include <cassert>

namespace teapot {

class TpRenderer {
public:
  TpRenderer(TpWindow &window, TpDevice &device);
  ~TpRenderer();

  TpRenderer(const TpRenderer &) = delete;
  TpRenderer &operator=(const TpRenderer &) = delete;

  bool isFrameInProgress() const { return isFrameStarted; }
  VkCommandBuffer getCurrentCommandBuffer() const {
    assert(isFrameStarted && "Can not obtain commandbuffer when there is no frame in progress");
    return commandBuffers[currentFrameIndex];
  }

  int getFrameIndex() const {
    assert(isFrameStarted && "Can not get this if frame is not in progress");
    return currentFrameIndex;
  }

  VkRenderPass getSwapChainRenderPass() const {
    return tpSwapChain->getRenderPass();
  }
  float getAspectRatio() const {
    return tpSwapChain->extentAspectRatio();
  }

  VkCommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();

  teapot::TpWindow &tpWindow;
  teapot::TpDevice &tpDevice;
  std::unique_ptr<teapot::TpSwapChain> tpSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;

  uint32_t currentImageIndex = 0;
  int currentFrameIndex = 0;
  bool isFrameStarted = false;
};

}

#endif //TEAPOT_TP_RENDERER_H
