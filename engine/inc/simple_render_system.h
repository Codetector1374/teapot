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
  SimpleRenderSystem(TpDevice &device, VkRenderPass renderPass);
  ~SimpleRenderSystem();

  SimpleRenderSystem(const SimpleRenderSystem &) = delete;
  SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

  void run();

  void renderGameObjects(int FrameIndex, VkCommandBuffer commandBuffer,
                         std::vector<TpGameObject> &gameObjects, const TpCamera &camera);
private:
  void createPipelineLayout();
  void createPipeline(VkRenderPass renderPass);

  void createDescriptorSetLayout();

  void updateUbo(VkCommandBuffer commandBuffer,
                 VkBuffer uboBuffer, VmaAllocation uboAllocation,
                 VkDescriptorSet descSet,
                 const TpGameObject &obj, const TpCamera &camera);


  teapot::TpDevice &tpDevice;
  std::unique_ptr<teapot::TpPipeline> tpPipeline;

  VkPipelineLayout pipelineLayout{};
  VkDescriptorSetLayout descriptorSetLayout{};
};
}  // namespace teapot

#endif //TEAPOT_SIMPLE_RENDER_SYSTEM_H
