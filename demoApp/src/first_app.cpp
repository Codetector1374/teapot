#include "first_app.h"
#include "simple_render_system.h"

// GLM Configuration
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <stdexcept>

namespace tpApp {
using namespace teapot;

FirstApp::FirstApp() {
  loadGameObjects();
}

FirstApp::~FirstApp() = default;

void FirstApp::run() {
  SimpleRenderSystem simpleRenderSystem{tpDevice,
                                        tpRenderer.getSwapChainRenderPass(), tpRenderer.getDescriptorSetLayout()};
  TpCamera camera{};
  camera.setViewDirection(glm::vec3{0.f, 0.f, 0.f}, glm::vec3{0.0, 0.f, 1.f});

  while (!tpWindow.shouldClose()) {
    glfwPollEvents();
    float aspect = tpRenderer.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 10.0f);

    if (auto commandBuffer = tpRenderer.beginFrame()) {
      tpRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer,
                                           tpRenderer.getCurrentUboBuffer(), tpRenderer.getCurrentUboAllocation(),
                                           tpRenderer.getCurrentDescriptorSet(),
                                           gameObjects, camera);
      tpRenderer.endSwapChainRenderPass(commandBuffer);
      tpRenderer.endFrame();
    }

  }

  vkDeviceWaitIdle(tpDevice.device());
}

// temporary helper function, creates a 1x1x1 cube centered at offset
std::unique_ptr<TpModel> createCubeModel(TpDevice& device, glm::vec3 offset) {
  std::vector<Vertex> vertices {
          {{0,0,0}, {1,0,0}},
          {{0.5,0,0}, {1,0,0}},
          {{0.5,0.5,0}, {1,0,0}},
          {{0.0,0.5,0}, {1,0,0}},

          {{0.25,0.25,1}, {1,1,0}},
          {{0.75,0.25,1}, {1,1,0}},
          {{0.75,0.75,1}, {1,1,0}},
          {{0.25,0.75,1}, {1,1,0}},
  };
  for (auto& v : vertices) {
    v.position += offset;
  }
  std::vector<uint32_t> indexLMAO {
    0,1,3,
    1,2,3,

    4,5,7,
    5,6,7,
  };
  return std::make_unique<TpModel>(device, vertices, indexLMAO);
}

void FirstApp::loadGameObjects() {
//  std::shared_ptr<TpModel> tpModel = createCubeModel(tpDevice, {0,0,0});
  std::shared_ptr<TpModel> tpModel = TpModel::loadObjFile(tpDevice, "../../demoApp/models/scene.obj");
  auto cube = TpGameObject::createGameObject();
  cube.model = tpModel;
  cube.transform.translation = {0,0,2};
  cube.transform.scale = {0.003,0.003,0.003};
  cube.transform.rotation.z = glm::radians<float>(180);
  gameObjects.push_back(std::move(cube));




}

}  // namespace teapot