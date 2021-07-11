//
// Created by user on 6/30/2021.
//

#ifndef TEAPOT_SIMPLE_RENDER_SYSTEM_H
#define TEAPOT_SIMPLE_RENDER_SYSTEM_H

#include "tp_device.h"
#include "tp_camera.h"
#include "tp_pipeline.h"
#include "tp_gameobject.h"
#include "tp_renderer.h"

// std
#include <memory>
#include <vector>

namespace teapot {

class SimpleRenderSystem {
public:
  SimpleRenderSystem(TpDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout descLayout);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  void run();

  void renderGameObjects(VkCommandBuffer commandBuffer,
                         VkBuffer uboBuffer, VmaAllocation uboAllocation,
                         VkDescriptorSet descSet,
                         std::vector<TpGameObject> &gameObjects, const TpCamera &camera);

private:
  void createPipelineLayout(VkDescriptorSetLayout descLayout);
  void createPipeline(VkRenderPass renderPass);

  void updateUbo(VkCommandBuffer commandBuffer,
                 VkBuffer uboBuffer, VmaAllocation uboAllocation,
                 VkDescriptorSet descSet,
                 const TpGameObject &obj, const TpCamera &camera);
  teapot::TpDevice &tpDevice;
  std::unique_ptr<teapot::TpPipeline> tpPipeline;

  VkPipelineLayout pipelineLayout{};
};
}  // namespace teapot

#endif //TEAPOT_SIMPLE_RENDER_SYSTEM_H
