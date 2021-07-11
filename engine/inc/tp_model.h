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
    glm::vec3 position;
    glm::vec3 color;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
  };

  TpModel(TpDevice &device, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
  ~TpModel();

  TpModel(const TpModel &) = delete;
  TpModel &operator=(const TpModel &) = delete;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);
  void createIndexBuffer(const std::vector<uint32_t> &indices);

  TpDevice& tpDevice;
  VkBuffer vertexBuffer;
  VmaAllocation vertexBufferAllocation;
  uint32_t vertexCount;

  VkBuffer indexBuffer;
  VmaAllocation indexBufferAllocation;
  uint32_t indexCount;
};
}

#endif //TEAPOT_TP_MODEL_H
