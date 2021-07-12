#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
namespace teapot {

class TpWindow {
 public:
  TpWindow(int w, int h, std::string name);
  ~TpWindow();

  TpWindow(const TpWindow &) = delete;
  TpWindow &operator=(const TpWindow &) = delete;

  bool shouldClose() { return glfwWindowShouldClose(window); }
  VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
  bool wasWindowResized() { return framebufferResized; }
  void resetWindowResizedFlag() { framebufferResized = false; }
  GLFWwindow *getWindow() const { return window; }
  void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

 private:
  void initWindow();
  static void framebufferResizeCallback(GLFWwindow *win, int w, int h);

  int width;
  int height;
  bool framebufferResized = false;

  std::string windowName;
  GLFWwindow *window;
};
}  // namespace teapot