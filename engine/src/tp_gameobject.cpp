//
// Created by devbox on 7/11/2021.
//
#include "tp_swap_chain.h"
#include "tp_gameobject.h"

#include <stdexcept>
#include <array>
#include <utility>

namespace teapot {

struct UniformBufferObject {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
};

TpGameObject::TpGameObject(TpGameObject::id_t objId,
                           TpDevice &tpDevice,
                           VkDescriptorSetLayout layout,
                           std::shared_ptr<TpModel> model) : id{objId}, tpDevice(tpDevice), model(std::move(model)) {
  createUniformBuffers();
  createDescriptorPool();
  createDescriptorSets(layout);
}

TpGameObject::~TpGameObject() {
  if (uniformBuffers.empty()) return;

  destroyDescriptorPool();
  freeUniformBuffers();
}

void TpGameObject::destroyDescriptorPool() {
  vkDestroyDescriptorPool(tpDevice.device(), descriptorPool, nullptr);
}

void TpGameObject::createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout) {
  std::vector<VkDescriptorSetLayout> layouts(TpSwapChain::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(TpSwapChain::MAX_FRAMES_IN_FLIGHT);
  allocInfo.pSetLayouts = layouts.data();

  descriptorSets.resize(TpSwapChain::MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(tpDevice.device(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate descriptor sets");
  }

  for (size_t i = 0; i < TpSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = model->textureImageView;
    imageInfo.sampler = model->textureSampler;

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(tpDevice.device(),
                           descriptorWrites.size(), descriptorWrites.data(),
                           0, nullptr);
  }
}

void TpGameObject::createUniformBuffers() {
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  uniformBuffers.resize(TpSwapChain::MAX_FRAMES_IN_FLIGHT);
  uniformBufferAllocations.resize(TpSwapChain::MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < TpSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
    tpDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VMA_MEMORY_USAGE_CPU_TO_GPU, uniformBuffers[i], uniformBufferAllocations[i]);
  }
}

void TpGameObject::freeUniformBuffers() {
  for (size_t i = 0; i < TpSwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
    vmaFreeMemory(tpDevice.allocator(), uniformBufferAllocations[i]);
  }
}


void TpGameObject::createDescriptorPool() {
  std::array<VkDescriptorPoolSize, 2> poolSizes{};
  // uniform buffer pool
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(TpSwapChain::MAX_FRAMES_IN_FLIGHT);
  // Texture Pool
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = static_cast<uint32_t>(TpSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = poolSizes.size();
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = TpSwapChain::MAX_FRAMES_IN_FLIGHT;

  if (vkCreateDescriptorPool(tpDevice.device(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to creating descriptor pool");
  }
}

void TpGameObject::bind(int frameIndex, VkPipelineLayout pipelineLayout, VkCommandBuffer buffer) {

  // UBO
  UniformBufferObject ubo{};
  ubo.model = transform.mat4();

  void *mappedUboData;
  vmaMapMemory(tpDevice.allocator(), uniformBufferAllocations[frameIndex], &mappedUboData);
  memcpy(mappedUboData, &ubo, sizeof(ubo));
  vmaUnmapMemory(tpDevice.allocator(), uniformBufferAllocations[frameIndex]);

  vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                          0, 1, &descriptorSets[frameIndex],
                          0, nullptr);

  model->bind(buffer);
}

void TpGameObject::draw(VkCommandBuffer buffer) {
  model->draw(buffer);
}

}