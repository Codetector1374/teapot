#include "tp_renderer.h"

// std
#include <array>
#include <stdexcept>

namespace teapot {

TpRenderer::TpRenderer(TpWindow &window, TpDevice &device): tpWindow{window}, tpDevice{device} {
  recreateSwapChain();

  createUniformBuffers();

  createDescriptorSetLayout();
  createDescriptorPool();
  createDescriptorSets();

  createCommandBuffers();
}

TpRenderer::~TpRenderer() {
  freeUniformBuffers();
  freeCommandBuffers();

  vkDestroyDescriptorSetLayout(tpDevice.device(), descriptorSetLayout, nullptr);
}

void TpRenderer::createCommandBuffers() {
  commandBuffers.resize(TpSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = tpDevice.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(tpDevice.device(), &allocInfo, commandBuffers.data()) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void TpRenderer::freeCommandBuffers() {
  vkFreeCommandBuffers(tpDevice.device(), tpDevice.getCommandPool(), commandBuffers.size(), commandBuffers.data());
  commandBuffers.clear();
}

void TpRenderer::createUniformBuffers() {
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  uniformBuffers.resize(TpSwapChain::MAX_FRAMES_IN_FLIGHT);
  uniformBufferAllocations.resize(TpSwapChain::MAX_FRAMES_IN_FLIGHT);

  for(size_t i = 0; i < TpSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
    tpDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VMA_MEMORY_USAGE_CPU_TO_GPU, uniformBuffers[i], uniformBufferAllocations[i]);
  }
}

void TpRenderer::freeUniformBuffers() {
  for(size_t i = 0; i < TpSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
    vmaFreeMemory(tpDevice.allocator(), uniformBufferAllocations[i]);
  }
}

void TpRenderer::createDescriptorPool() {
  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSize.descriptorCount = TpSwapChain::MAX_FRAMES_IN_FLIGHT;

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;
  poolInfo.maxSets = TpSwapChain::MAX_FRAMES_IN_FLIGHT;

  if (vkCreateDescriptorPool(tpDevice.device(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to creating descriptor pool");
  }
}

void TpRenderer::destroyDescriptorPool() {
  vkDestroyDescriptorPool(tpDevice.device(), descriptorPool, nullptr);
}

void TpRenderer::createDescriptorSets() {
  std::vector<VkDescriptorSetLayout> layouts(TpSwapChain::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(TpSwapChain::MAX_FRAMES_IN_FLIGHT);
  allocInfo.pSetLayouts = layouts.data();

  descriptorSets.resize(TpSwapChain::MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(tpDevice.device(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate descriptor sets");
  }

  for (size_t i = 0; i < TpSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSets[i];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr; // Optional
    descriptorWrite.pTexelBufferView = nullptr; // Optional

    vkUpdateDescriptorSets(tpDevice.device(), 1, &descriptorWrite, 0, nullptr);
  }
}

void TpRenderer::createDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &uboLayoutBinding;

  if (vkCreateDescriptorSetLayout(tpDevice.device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor layout");
  }
}

void TpRenderer::recreateSwapChain() {
  auto extent = tpWindow.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = tpWindow.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(tpDevice.device());
  if (tpSwapChain == nullptr) {
    tpSwapChain = std::make_unique<TpSwapChain>(tpDevice, extent);
  } else {
    std::shared_ptr<TpSwapChain> oldChain = std::move(tpSwapChain);
    tpSwapChain = std::make_unique<TpSwapChain>(tpDevice, extent, oldChain);
    if (!oldChain->compareSwapFormats(*tpSwapChain)) {
      throw std::runtime_error("Swap chain image(or color depth) has changed");
    }
  }
}

VkCommandBuffer TpRenderer::beginFrame() {
  assert(!isFrameStarted && "can't call if already in progress");
  auto result = tpSwapChain->acquireNextImage(&currentImageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return nullptr;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  isFrameStarted = true;

  auto commandBuffer = getCurrentCommandBuffer();
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }
  return commandBuffer;
}

void TpRenderer::endFrame() {
  assert(isFrameStarted && "can't end while frame is not in progress");
  auto commandBuffer = getCurrentCommandBuffer();
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }

  auto result = tpSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || tpWindow.wasWindowResized()) {
    tpWindow.resetWindowResizedFlag();
    recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  isFrameStarted = false;
  currentFrameIndex = (currentFrameIndex + 1) % TpSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void TpRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted && "Gotta start frame");
  assert(commandBuffer == getCurrentCommandBuffer() && "can't draw on a different frame");

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = tpSwapChain->getRenderPass();
  renderPassInfo.framebuffer = tpSwapChain->getFrameBuffer(static_cast<int>(currentImageIndex));

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = tpSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(tpSwapChain->getSwapChainExtent().width);
  viewport.height = static_cast<float>(tpSwapChain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, tpSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void TpRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  vkCmdEndRenderPass(commandBuffer);
}


}  // namespace teapot