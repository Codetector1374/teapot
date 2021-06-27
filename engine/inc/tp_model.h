//
// Created by user on 6/27/2021.
//

#ifndef TEAPOT_TP_MODEL_H
#define TEAPOT_TP_MODEL_H

#include "tp_device.h"

// GLM Configuration
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace teapot {
class TpModel {
public:

  struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
  };

  TpModel(TpDevice &device, const std::vector<Vertex> &vertices);
  ~TpModel();

  TpModel(const TpModel &) = delete;
  TpModel &operator=(const TpModel &) = delete;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);

  TpDevice& tpDevice;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  uint32_t vertexCount;
};
}

#endif //TEAPOT_TP_MODEL_H
