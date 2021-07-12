#include "simple_render_system.h"

// std
#include <array>
#include <chrono>

#include <stdexcept>

namespace teapot {

struct SimplePushConstantData {
  glm::mat4 proj{1.f};
  glm::mat4 view{1.f};
};

SimpleRenderSystem::SimpleRenderSystem(TpDevice &device, VkRenderPass renderPass): tpDevice{device} {
  createDescriptorSetLayout();
  createPipelineLayout();
  createPipeline(renderPass);
}


void SimpleRenderSystem::createDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.pImmutableSamplers = nullptr;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = bindings.size();
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(tpDevice.device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor layout");
  }
}

SimpleRenderSystem::~SimpleRenderSystem() {
  vkDestroyPipelineLayout(tpDevice.device(), pipelineLayout, nullptr);
  vkDestroyDescriptorSetLayout(tpDevice.device(), descriptorSetLayout, nullptr);
}

void SimpleRenderSystem::createPipelineLayout() {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.size = sizeof(SimplePushConstantData);
  pushConstantRange.offset = 0;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(tpDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
  PipelineConfigInfo pipelineConfig{};
  TpPipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  tpPipeline = std::make_unique<TpPipeline>(
          tpDevice,
          "assets/shaders/simple_shader.vert.spv",
          "assets/shaders/simple_shader.frag.spv",
          pipelineConfig);
}

void SimpleRenderSystem::renderGameObjects(int frameIndex, VkCommandBuffer commandBuffer,
                                           std::vector<TpGameObject> &gameObjects, const teapot::TpCamera &camera) {
  tpPipeline->bind(commandBuffer);

  for(auto& obj: gameObjects) {
    obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
//    obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.005f, glm::two_pi<float>());

    SimplePushConstantData push{};
    push.proj = camera.getProjection();
    push.view = camera.getView();

    vkCmdPushConstants(commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0,
                       sizeof(SimplePushConstantData),
                       &push);

//    updateUbo(commandBuffer, uboBuffer, uboAllocation, descSet, obj, camera);

    obj.bind(frameIndex, pipelineLayout, commandBuffer);
    obj.draw(commandBuffer);
  }
}

}  // namespace teapot