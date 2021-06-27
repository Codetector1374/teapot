#include "tp_window.h"

// std
#include <stdexcept>

namespace teapot {

TpWindow::TpWindow(int w, int h, std::string name) : width{w}, height{h}, windowName{name} {
  initWindow();
}

TpWindow::~TpWindow() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void TpWindow::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}

void TpWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
  if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to craete window surface");
  }
}
}  // namespace teapot