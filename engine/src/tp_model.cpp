//
// Created by user on 6/27/2021.
//
#include "tp_model.h"

#include <cstring>
#include <cassert>
#include <stdexcept>

#include "tiny_obj_loader.h"

namespace teapot {

TpModel::TpModel(TpDevice &device, const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices): tpDevice(device) {
  createVertexBuffers(vertices);
  createIndexBuffer(indices);
}

std::shared_ptr<TpModel> TpModel::loadObjFile(TpDevice &device, std::string objFilePath) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objFilePath.c_str())) {
    throw std::runtime_error(err);
  }

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      Vertex vertex{};

      vertex.position = {
              attrib.vertices[3 * index.vertex_index + 0],
              attrib.vertices[3 * index.vertex_index + 1],
              attrib.vertices[3 * index.vertex_index + 2]
      };

//      vertex.texCoord = {
//              attrib.texcoords[2 * index.texcoord_index + 0],
//              attrib.texcoords[2 * index.texcoord_index + 1]
//      };

      vertex.color = {0.f, 0.5f, 1.0f};

      vertices.push_back(vertex);
      indices.push_back(indices.size());
    }
  }

  return std::make_shared<TpModel>(device, vertices, indices);
}

TpModel::~TpModel() {
  vmaDestroyBuffer(tpDevice.allocator(), vertexBuffer, vertexBufferAllocation);
  vmaDestroyBuffer(tpDevice.allocator(), indexBuffer, indexBufferAllocation);
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
                        vertexBuffer, vertexBufferAllocation);
  tpDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
  vmaDestroyBuffer(tpDevice.allocator(), stagingBuffer, stagingAlloc);
}

void TpModel::createIndexBuffer(const std::vector<uint32_t> &indices) {
  indexCount = static_cast<uint32_t>(indices.size());
  VkDeviceSize indexBufferSize = sizeof(indices[0]) * indexCount;

  VkBuffer stagingBuffer;
  VmaAllocation stagingBufferAllocation;
  tpDevice.createBuffer(indexBufferSize,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VMA_MEMORY_USAGE_CPU_TO_GPU,
                        stagingBuffer, stagingBufferAllocation);

  void *data;
  vmaMapMemory(tpDevice.allocator(), stagingBufferAllocation, &data);
  memcpy(data, indices.data(), (size_t) indexBufferSize);
  vmaUnmapMemory(tpDevice.allocator(), stagingBufferAllocation);

  tpDevice.createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                        VMA_MEMORY_USAGE_GPU_ONLY, indexBuffer, indexBufferAllocation);

  tpDevice.copyBuffer(stagingBuffer, indexBuffer, indexBufferSize);
  vmaDestroyBuffer(tpDevice.allocator(), stagingBuffer, stagingBufferAllocation);
}

/*
 * bind the vertex buffer with binding / binding descriptions
 */
void TpModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
  vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void TpModel::draw(VkCommandBuffer commandBuffer) {
  vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}


std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);

  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions() {
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

