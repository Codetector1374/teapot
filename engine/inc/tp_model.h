//
// Created by user on 6/27/2021.
//

#ifndef TEAPOT_TP_MODEL_H
#define TEAPOT_TP_MODEL_H

#include "tp_device.h"

#include <memory>

// GLM Configuration
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace teapot {
struct Vertex {
  glm::vec3 position;
  glm::vec3 color;
  glm::vec2 texCoord;

  static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};

class TpModel {
public:
  TpModel(TpDevice &device,
          const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
          const std::string &texture);
  ~TpModel();

  static std::shared_ptr<TpModel> loadObjFile(TpDevice &device,
                                              const std::string& objFilePath, const std::string &texturePath);

  TpModel(const TpModel &) = delete;
  TpModel &operator=(const TpModel &) = delete;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);
  void createIndexBuffer(const std::vector<uint32_t> &indices);

  void createTextureImage(const std::string& imagePath);
  void createTextureImageView();
  void createTextureSampler();


  TpDevice& tpDevice;
  VkBuffer vertexBuffer;
  VmaAllocation vertexBufferAllocation;

  uint32_t vertexCount;
  VkBuffer indexBuffer;
  VmaAllocation indexBufferAllocation;

  uint32_t indexCount;
  VkImage textureImage = nullptr;
  VmaAllocation textureImageAllocation = nullptr;
public:
  VkImageView textureImageView;
  VkSampler textureSampler;
private:
};
}

#endif //TEAPOT_TP_MODEL_H
