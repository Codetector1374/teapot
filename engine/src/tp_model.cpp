//
// Created by user on 6/27/2021.
//
#include "tp_model.h"

#include <cstring>
#include <cassert>

namespace teapot {

TpModel::TpModel(TpDevice &device, const std::vector<Vertex> &vertices): tpDevice(device) {
  createVertexBuffers(vertices);
}

TpModel::~TpModel() {
  vmaDestroyBuffer(tpDevice.allocator(), vertexBuffer, bufferAlloc);
}

void TpModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

  VkBuffer stagingBuffer;
  VmaAllocation stagingAlloc;
  tpDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VMA_MEMORY_USAGE_CPU_TO_GPU,
                        stagingBuffer, stagingAlloc);
  void *data;
  vmaMapMemory(tpDevice.allocator(), stagingAlloc, &data);
  memcpy(data, vertices.data(), bufferSize);
  vmaFlushAllocation(tpDevice.allocator(), stagingAlloc, 0, bufferSize);
  vmaUnmapMemory(tpDevice.allocator(), stagingAlloc);

  tpDevice.createBuffer(bufferSize,
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                        VMA_MEMORY_USAGE_GPU_ONLY,
                        vertexBuffer, bufferAlloc);
  tpDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
  vmaDestroyBuffer(tpDevice.allocator(), stagingBuffer, stagingAlloc);
}

/*
 * bind the vertex buffer with binding / binding descriptions
 */
void TpModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void TpModel::draw(VkCommandBuffer commandBuffer) {
  vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
}

std::vector<VkVertexInputBindingDescription> TpModel::Vertex::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> TpModel::Vertex::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attrDescs(2);

  attrDescs[0].binding = 0;
  attrDescs[0].location = 0;
  attrDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attrDescs[0].offset = offsetof(Vertex, position);

  attrDescs[1].binding = 0;
  attrDescs[1].location = 1;
  attrDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attrDescs[1].offset = offsetof(Vertex, color);

  return attrDescs;
}


}

