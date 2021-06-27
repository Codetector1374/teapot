#include "first_app.h"

// std
#include <array>
#include <stdexcept>

namespace tpApp {
using namespace teapot;
FirstApp::FirstApp() {
  loadModel();
  createPipelineLayout();
  recreateSwapChain();
  createCommandBuffers();
}

FirstApp::~FirstApp() { vkDestroyPipelineLayout(tpDevice.device(), pipelineLayout, nullptr); }

void FirstApp::run() {
  while (!tpWindow.shouldClose()) {
    glfwPollEvents();
    drawFrame();
  }

  vkDeviceWaitIdle(tpDevice.device());
}

void FirstApp::createPipelineLayout() {
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;
  if (vkCreatePipelineLayout(tpDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void FirstApp::createPipeline() {
  PipelineConfigInfo pipelineConfig{};
  TpPipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = tpSwapChain->getRenderPass();
  pipelineConfig.pipelineLayout = pipelineLayout;
  tpPipeline = std::make_unique<TpPipeline>(
          tpDevice,
          "assets/shaders/simple_shader.vert.spv",
          "assets/shaders/simple_shader.frag.spv",
          pipelineConfig);
}

void FirstApp::createCommandBuffers() {
  commandBuffers.resize(tpSwapChain->imageCount());

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



void FirstApp::drawFrame() {
  uint32_t imageIndex;
  auto result = tpSwapChain->acquireNextImage(&imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  recordCommandBuffer(imageIndex);
  result = tpSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || tpWindow.wasWindowResized()) {
    tpWindow.resetWindowResizedFlag();
    recreateSwapChain();
    return;
  }
  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }
}

void FirstApp::loadModel() {
  std::vector<TpModel::Vertex> vertices {
          {{0, -0.5}, {1,0,0}},
          {{0.5,0.5}, {0,1,0}},
          {{-0.5,0.5}, {0,0,1}},
  };

  tpModel = std::make_unique<TpModel>(tpDevice, vertices);
}

void FirstApp::recordCommandBuffer(uint32_t imageIndex) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = tpSwapChain->getRenderPass();
  renderPassInfo.framebuffer = tpSwapChain->getFrameBuffer(imageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = tpSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(tpSwapChain->getSwapChainExtent().width);
  viewport.height = static_cast<float>(tpSwapChain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, tpSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
  vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

  tpPipeline->bind(commandBuffers[imageIndex]);
  tpModel->bind(commandBuffers[imageIndex]);
  tpModel->draw(commandBuffers[imageIndex]);

  vkCmdEndRenderPass(commandBuffers[imageIndex]);
  if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}

void FirstApp::recreateSwapChain() {
  auto extent = tpWindow.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = tpWindow.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(tpDevice.device());
  if (tpSwapChain == nullptr) {
    tpSwapChain = std::make_unique<TpSwapChain>(tpDevice, extent);
  } else {
    tpSwapChain = std::make_unique<TpSwapChain>(tpDevice, extent, std::move(tpSwapChain));
    if (tpSwapChain->imageCount() != commandBuffers.size()) {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }

  // Check if we _really_ need to recreate pipeline
  createPipeline();
}

void FirstApp::freeCommandBuffers() {
  vkFreeCommandBuffers(tpDevice.device(), tpDevice.getCommandPool(), commandBuffers.size(), commandBuffers.data());
  commandBuffers.clear();
}

}  // namespace teapot