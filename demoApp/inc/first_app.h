#pragma once

#include "tp_device.h"
#include "tp_renderer.h"
#include "tp_window.h"
#include "tp_gameobject.h"

// std
#include <memory>
#include <vector>

namespace tpApp {

using namespace teapot;

class FirstApp {
 public:
  static constexpr int WIDTH = 800;
  static constexpr int HEIGHT = 600;

  FirstApp();
  ~FirstApp();

  FirstApp(const FirstApp &) = delete;
  FirstApp &operator=(const FirstApp &) = delete;

  void run();

 private:
  void loadGameObjects();

  teapot::TpWindow tpWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
  teapot::TpDevice tpDevice{tpWindow};
  teapot::TpRenderer tpRenderer{tpWindow, tpDevice};

  std::vector<TpGameObject> gameObjects;
};
}  // namespace teapot