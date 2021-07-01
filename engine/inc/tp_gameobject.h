//
// Created by user on 6/29/2021.
//
#ifndef TEAPOT_TP_GAMEOBJECT_H
#define TEAPOT_TP_GAMEOBJECT_H

#include "tp_model.h"
#include <memory>

namespace teapot {
struct Transform2dComponent {
  glm::vec2 translation{};
  glm::vec2 scale{1.f, 1.f};
  float rotation;

  glm::mat2 mat2() const {
    const float sin = glm::sin(rotation);
    const float cos = glm::cos(rotation);
    glm::mat2 rotMat{{cos, sin}, {-sin, cos}};
    glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
    return rotMat * scaleMat;
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
    Transform2dComponent transform2d{};
  private:

    TpGameObject(id_t objId): id{objId} {}
    id_t id;

  };
}

#endif //TEAPOT_TP_GAMEOBJECT_H
