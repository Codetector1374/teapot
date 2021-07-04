//
// Created by user on 6/29/2021.
//
#ifndef TEAPOT_TP_GAMEOBJECT_H
#define TEAPOT_TP_GAMEOBJECT_H

#include "tp_model.h"
#include <memory>

#include <glm/gtc/matrix_transform.hpp>

namespace teapot {
struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.f, 1.f, 1.f};
  glm::vec3 rotation;

  glm::mat4 mat4() {
    auto transform = glm::translate(glm::mat4{1.f}, translation);

    // Apply three axis of rotation in order
    transform = glm::rotate(transform, rotation.y, {0,1,0});
    transform = glm::rotate(transform, rotation.x, {1,0,0});
    transform = glm::rotate(transform, rotation.z, {0,0,1});

    transform = glm::scale(transform, scale);
    return transform;
  }
};

  class TpGameObject {
  public:
    using id_t = unsigned int;

    static TpGameObject createGameObject() {
      static id_t currentId = 0;
      return TpGameObject{currentId++};
    }

    id_t getId() const {
      return id;
    }

    TpGameObject(const TpGameObject &) = delete;
    TpGameObject &operator=(const TpGameObject&) = delete;

    TpGameObject(TpGameObject &&) = default;
    TpGameObject &operator=(TpGameObject &&) = default;

    std::shared_ptr<TpModel> model{};
    glm::vec3 color{};
    TransformComponent transform{};
  private:

    TpGameObject(id_t objId): id{objId} {}
    id_t id;

  };
}

#endif //TEAPOT_TP_GAMEOBJECT_H
