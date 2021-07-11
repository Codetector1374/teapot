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

// GLM Configuration
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace teapot {

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

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

  VkBuffer getCurrentUniformBuffer() const {
    assert(isFrameStarted && "Frame needs to start");
    return uniformBuffers[currentFrameIndex];
  }

  VkBuffer getCurrentUboBuffer() const {
    return uniformBuffers[currentFrameIndex];
  }

  VmaAllocation getCurrentUboAllocation() const {
    return uniformBufferAllocations[currentFrameIndex];
  }

  VkDescriptorSet getCurrentDescriptorSet() const {
    return descriptorSets[currentFrameIndex];
  }

  int getFrameIndex() const {
    assert(isFrameStarted && "Can not get this if frame is not in progress");
    return currentFrameIndex;
  }

  VkRenderPass getSwapChainRenderPass() const {
    return tpSwapChain->getRenderPass();
  }

  VkDescriptorSetLayout getDescriptorSetLayout() const {
    return descriptorSetLayout;
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
  void createUniformBuffers();
  void freeUniformBuffers();

  void createDescriptorPool();
  void createDescriptorSets();
  void destroyDescriptorPool();
  void createDescriptorSetLayout();

  void recreateSwapChain();
  teapot::TpWindow &tpWindow;

  teapot::TpDevice &tpDevice;
  std::unique_ptr<teapot::TpSwapChain> tpSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VmaAllocation> uniformBufferAllocations;

  VkDescriptorSetLayout descriptorSetLayout{};
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;

  uint32_t currentImageIndex = 0;
  int currentFrameIndex = 0;

  bool isFrameStarted = false;
};

}

#endif //TEAPOT_TP_RENDERER_H
