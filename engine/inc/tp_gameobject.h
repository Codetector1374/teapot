//
// Created by user on 6/29/2021.
//
#ifndef TEAPOT_TP_GAMEOBJECT_H
#define TEAPOT_TP_GAMEOBJECT_H

#include "tp_model.h"
#include <memory>
#include <optional>

#include <glm/gtc/matrix_transform.hpp>


namespace teapot {
struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.f, 1.f, 1.f};
  glm::vec3 rotation;

  glm::mat4 mat4() const {
    auto transform = glm::translate(glm::mat4{1.f}, translation);

    // Apply three axis of rotation in order
    transform = glm::rotate(transform, rotation.y, {0, 1, 0});
    transform = glm::rotate(transform, rotation.x, {1, 0, 0});
    transform = glm::rotate(transform, rotation.z, {0, 0, 1});

    transform = glm::scale(transform, scale);
    return transform;
  }
};

class TpGameObject {
public:
  using id_t = unsigned int;

  static TpGameObject createGameObject(TpDevice &device, VkDescriptorSetLayout layout, std::shared_ptr<TpModel> model) {
    static id_t currentId = 1;
    return TpGameObject{currentId++, device, layout, model};
  }

  id_t getId() const {
    return id;
  }

  TpGameObject(const TpGameObject &) = delete;
  TpGameObject &operator=(const TpGameObject &) = delete;
  TpGameObject(TpGameObject &&) = default;
  TpGameObject &operator=(TpGameObject &&) = default;
  ~TpGameObject();

  void bind(int frameIndex, VkPipelineLayout pipelineLayout, VkCommandBuffer buffer);
  void draw(VkCommandBuffer buffer);

  std::shared_ptr<TpModel> model;
  glm::vec3 color{};
  TransformComponent transform{};
private:

  TpGameObject(id_t objId, TpDevice &tpDevice, VkDescriptorSetLayout layout, std::shared_ptr<TpModel> model);
  void createDescriptorPool();
  void createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout);
  void destroyDescriptorPool();
  void createUniformBuffers();

  void freeUniformBuffers();

  id_t id;
  TpDevice& tpDevice;

  VkDescriptorPool descriptorPool{};
  std::vector<VkDescriptorSet> descriptorSets;

  std::vector<VkBuffer> uniformBuffers;
  std::vector<VmaAllocation> uniformBufferAllocations;
};
}

#endif //TEAPOT_TP_GAMEOBJECT_H
