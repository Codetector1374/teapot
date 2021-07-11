#include "simple_render_system.h"

// std
#include <array>
#include <chrono>

#include <stdexcept>

namespace teapot {

struct SimplePushConstantData {
  glm::mat4 transform{1.f};
  alignas(16) glm::vec3 color;
};

SimpleRenderSystem::SimpleRenderSystem(TpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout descLayout): tpDevice{device} {
  createPipelineLayout(descLayout);
  createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
  vkDestroyPipelineLayout(tpDevice.device(), pipelineLayout, nullptr);
}

void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout descLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.size = sizeof(SimplePushConstantData);
  pushConstantRange.offset = 0;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &descLayout;
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

void SimpleRenderSystem::updateUbo(VkCommandBuffer commandBuffer,
                                   VkBuffer uboBuffer, VmaAllocation uboAllocation,
                                   VkDescriptorSet descSet,
                                   const TpGameObject &obj, const TpCamera &camera) {
  UniformBufferObject ubo{};
  ubo.model = obj.transform.mat4();
  ubo.proj = camera.getProjection();
  ubo.view = camera.getView();

  void *mappedUboData;
  vmaMapMemory(tpDevice.allocator(), uboAllocation, &mappedUboData);
  memcpy(mappedUboData, &ubo, sizeof(ubo));
  vmaUnmapMemory(tpDevice.allocator(), uboAllocation);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                          0, 1, &descSet,
                          0, nullptr);
}

void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer,
                                           VkBuffer uboBuffer, VmaAllocation uboAllocation,
                                           VkDescriptorSet descSet,
                                           std::vector<TpGameObject> &gameObjects, const teapot::TpCamera &camera) {
  tpPipeline->bind(commandBuffer);

  auto projectionView = camera.getView();

  for(auto& obj: gameObjects) {
//    obj.transform.rotation = glm::mod(obj.transform.rotation + 0.05f, glm::two_pi<float>());
    obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
    obj.transform.rotation.z = glm::mod(obj.transform.rotation.z + 0.01f, glm::two_pi<float>());

    SimplePushConstantData push{};
    push.color = obj.color;
    push.transform = projectionView * obj.transform.mat4();

    vkCmdPushConstants(commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0,
                       sizeof(SimplePushConstantData),
                       &push);

    updateUbo(commandBuffer, uboBuffer, uboAllocation, descSet, obj, camera);

    obj.model->bind(commandBuffer);
    obj.model->draw(commandBuffer);
  }
}

}  // namespace teapot