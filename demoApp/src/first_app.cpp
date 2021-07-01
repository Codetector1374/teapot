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

FirstApp::~FirstApp() {

}

void FirstApp::run() {
  SimpleRenderSystem simpleRenderSystem{tpDevice, tpRenderer.getSwapChainRenderPass()};

  while (!tpWindow.shouldClose()) {
    glfwPollEvents();

    if (auto commandBuffer = tpRenderer.beginFrame()) {
      tpRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
      tpRenderer.endSwapChainRenderPass(commandBuffer);
      tpRenderer.endFrame();
    }

  }

  vkDeviceWaitIdle(tpDevice.device());
}

void FirstApp::loadGameObjects() {
  std::vector<TpModel::Vertex> vertices {
          {{0, -0.5}, {1,0,0}},
          {{0.5,0.5}, {0,1,0}},
          {{-0.5,0.5}, {0,0,1}},
  };

  auto tpModel = std::make_shared<TpModel>(tpDevice, vertices);
  auto triangle = TpGameObject::createGameObject();
  triangle.model = tpModel;
  triangle.color = {.1, .8, .1};
//  triangle.transform2d.scale = {1.f, 1.f};
//  triangle.transform2d.rotation = .25f * glm::two_pi<float>();

  gameObjects.push_back(std::move(triangle));
}

}  // namespace teapot