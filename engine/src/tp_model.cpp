//
// Created by user on 6/27/2021.
//
#include "tp_model.h"

#include <cstring>
#include <cassert>
#include <stdexcept>
#include <array>
#include <tp_swap_chain.h>

#include "tiny_obj_loader.h"
#include "stb_image.h"

namespace teapot {


TpModel::TpModel(TpDevice &device,
                 const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
                 const std::string &texture): tpDevice(device) {
  createTextureImage(texture);
  if (textureImage != nullptr) {
    createTextureImageView();
    createTextureSampler();
  }
  createVertexBuffers(vertices);
  createIndexBuffer(indices);
}

std::shared_ptr<TpModel> TpModel::loadObjFile(TpDevice &device,
                                              const std::string& objFilePath,
                                              const std::string &texturePath) {
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

      vertex.texCoord = {
              attrib.texcoords[2 * index.texcoord_index + 0],
              -attrib.texcoords[2 * index.texcoord_index + 1]
      };

      vertex.color = {0.f, 0.5f, 1.0f};

      vertices.push_back(vertex);
      indices.push_back(indices.size());
    }
  }

  return std::make_shared<TpModel>(device, vertices, indices, texturePath);
}

TpModel::~TpModel() {
  vmaDestroyBuffer(tpDevice.allocator(), vertexBuffer, vertexBufferAllocation);
  vmaDestroyBuffer(tpDevice.allocator(), indexBuffer, indexBufferAllocation);
  if (textureImage != nullptr) {
    vmaDestroyImage(tpDevice.allocator(), textureImage, textureImageAllocation);
    vkDestroyImageView(tpDevice.device(), textureImageView, nullptr);
    vkDestroySampler(tpDevice.device(), textureSampler, nullptr);
  }
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

void TpModel::createTextureImage(const std::string& imagePath) {
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    printf("Failed to load texture\n");
    return;
  }

  VkBuffer stagingBuffer;
  VmaAllocation stagingBufferAllocation;
  tpDevice.createBuffer(imageSize,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VMA_MEMORY_USAGE_CPU_TO_GPU,
                        stagingBuffer, stagingBufferAllocation);

  void *data;
  vmaMapMemory(tpDevice.allocator(), stagingBufferAllocation, &data);
  memcpy(data, pixels, (size_t) imageSize);
  vmaUnmapMemory(tpDevice.allocator(), stagingBufferAllocation);
  stbi_image_free(pixels);

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = static_cast<uint32_t>(texWidth);
  imageInfo.extent.height = static_cast<uint32_t>(texHeight);
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0; // Optional

  tpDevice.createImageWithInfo(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, textureImage, textureImageAllocation);

  tpDevice.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  tpDevice.copyBufferToImage(stagingBuffer, textureImage, texWidth, texHeight, 1);
  tpDevice.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
                                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vmaDestroyBuffer(tpDevice.allocator(), stagingBuffer, stagingBufferAllocation);
}

void TpModel::createTextureImageView() {
  textureImageView = tpDevice.createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB);
}

void TpModel::createTextureSampler() {
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = 8;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;
  if (vkCreateSampler(tpDevice.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture sampler!");
  }
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
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(Vertex, position);

  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(Vertex, color);

  attributeDescriptions[2].binding = 0;
  attributeDescriptions[2].location = 2;
  attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
  attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

  return attributeDescriptions;
}


}

