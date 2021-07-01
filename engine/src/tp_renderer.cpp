#include "tp_renderer.h"

// std
#include <array>
#include <stdexcept>

namespace teapot {

TpRenderer::TpRenderer(TpWindow &window, TpDevice &device): tpWindow{window}, tpDevice{device} {
  recreateSwapChain();
  createCommandBuffers();
}

TpRenderer::~TpRenderer() {
  freeCommandBuffers();
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

void TpRenderer::freeCommandBuffers() {
  vkFreeCommandBuffers(tpDevice.device(), tpDevice.getCommandPool(), commandBuffers.size(), commandBuffers.data());
  commandBuffers.clear();
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
  renderPassInfo.framebuffer = tpSwapChain->getFrameBuffer(currentImageIndex);

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