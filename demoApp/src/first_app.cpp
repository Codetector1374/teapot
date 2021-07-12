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
}

FirstApp::~FirstApp() = default;

void FirstApp::run() {
  SimpleRenderSystem simpleRenderSystem{tpDevice,
                                        tpRenderer.getSwapChainRenderPass()};
  TpCamera camera{};
  camera.setViewDirection(glm::vec3{0.f, 0.f, 0.f}, glm::vec3{0.0, 0.f, 1.f});

  loadGameObjects(simpleRenderSystem.getDescriptorSetLayout());

  while (!tpWindow.shouldClose()) {
    glfwPollEvents();
    float aspect = tpRenderer.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 10.0f);


    if (glfwGetKey(tpWindow.getWindow(), GLFW_KEY_W) == GLFW_PRESS) {
      gameObjects[0].transform.translation.y -= 0.01;
    } else if (glfwGetKey(tpWindow.getWindow(), GLFW_KEY_S) == GLFW_PRESS) {
      gameObjects[0].transform.translation.y += 0.01;
    } else if (glfwGetKey(tpWindow.getWindow(), GLFW_KEY_LEFT) == GLFW_PRESS) {
      gameObjects[0].transform.rotation.y = glm::mod(gameObjects[0].transform.rotation.y + 0.01f, glm::two_pi<float>());
    }

    if (auto commandBuffer = tpRenderer.beginFrame()) {
      tpRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(tpRenderer.getFrameIndex(), commandBuffer,
                                           gameObjects, camera);
      tpRenderer.endSwapChainRenderPass(commandBuffer);
      tpRenderer.endFrame();
    }

  }

  vkDeviceWaitIdle(tpDevice.device());
}

// temporary helper function, creates a 1x1x1 cube centered at offset
//std::unique_ptr<TpModel> createCubeModel(TpDevice& device, glm::vec3 offset) {
//  std::vector<Vertex> vertices {
//          {{0,0,0}, {1,0,0}},
//          {{0.5,0,0}, {1,0,0}},
//          {{0.5,0.5,0}, {1,0,0}},
//          {{0.0,0.5,0}, {1,0,0}},
//
//          {{0.25,0.25,1}, {1,1,0}},
//          {{0.75,0.25,1}, {1,1,0}},
//          {{0.75,0.75,1}, {1,1,0}},
//          {{0.25,0.75,1}, {1,1,0}},
//  };
//  for (auto& v : vertices) {
//    v.position += offset;
//  }
//  std::vector<uint32_t> indexLMAO {
//    0,1,3,
//    1,2,3,
//
//    4,5,7,
//    5,6,7,
//  };
//  return std::make_unique<TpModel>(device, vertices, indexLMAO, "");
//}

void FirstApp::loadGameObjects(VkDescriptorSetLayout layout) {
//  std::shared_ptr<TpModel> tpModel = createCubeModel(tpDevice, {0,0,0});
  std::shared_ptr<TpModel> tpModel = TpModel::loadObjFile(tpDevice, "../../demoApp/models/chest/chest.obj",
                                                          "../../demoApp/models/chest/Scene_-_Root_baseColor.png");
  auto cube = TpGameObject::createGameObject(tpDevice, layout, tpModel);
  cube.transform.translation = {0,-0.5,2};
  cube.transform.scale = {0.2,0.2,0.2};
  cube.transform.rotation.z = glm::radians<float>(180);
  gameObjects.push_back(std::move(cube));

  auto roomModel = TpModel::loadObjFile(tpDevice, "../../demoApp/models/room/room.obj",
                                        "../../demoApp/models/room/room.png");
  auto cube2 = TpGameObject::createGameObject(tpDevice, layout, roomModel);
  cube2.transform.translation = {-1.7,1,4};
  cube2.transform.scale = {1,1,1};
  cube2.transform.rotation.x = glm::radians<float>(90);
  gameObjects.push_back(std::move(cube2));
}

}  // namespace teapot