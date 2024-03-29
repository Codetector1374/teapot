#pragma once

#include "tp_window.h"
#include <vk_mem_alloc.h>
// std lib headers
#include <string>
#include <vector>

namespace teapot {

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
  uint32_t graphicsFamily;
  uint32_t presentFamily;
  bool graphicsFamilyHasValue = false;
  bool presentFamilyHasValue = false;
  bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

class TpDevice {
 public:
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  TpDevice(TpWindow &window);
  ~TpDevice();

  // Not copyable or movable
  TpDevice(const TpDevice &) = delete;
  TpDevice operator=(const TpDevice &) = delete;
  TpDevice(TpDevice &&) = delete;
  TpDevice &operator=(TpDevice &&) = delete;

  VkCommandPool getCommandPool() const { return commandPool; }
  VkDevice device() const { return device_; }
  VkSurfaceKHR surface() const { return surface_; }
  VkQueue graphicsQueue() const { return graphicsQueue_; }
  VkQueue presentQueue() const { return presentQueue_; }
  VmaAllocator allocator() const { return allocator_; }

  SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevices[0]); }
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
  QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevices[0]); }
  VkFormat findSupportedFormat(
      const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

  // Buffer Helper Functions
  void createBuffer(
      VkDeviceSize size,
      VkBufferUsageFlags usage,
      VmaMemoryUsage vmaUsage,
      VkBuffer &buffer,
      VmaAllocation &allocation);
  VkImageView createImageView(VkImage image, VkFormat format);

  VkCommandBuffer beginSingleTimeCommands();
  void endSingleTimeCommands(VkCommandBuffer commandBuffer);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void copyBufferToImage(
      VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
  void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

  void createImageWithInfo(
      const VkImageCreateInfo &imageInfo,
      VmaMemoryUsage properties,
      VkImage &image,
      VmaAllocation &imageAllocation);

  VkPhysicalDeviceProperties properties{};

 private:
  void createInstance();
  void setupDebugMessenger();
  void createSurface();
  void pickPhysicalDevice();
  void createLogicalDevice();
  void createCommandPool();

  // helper functions
  bool isDeviceSuitable(VkPhysicalDevice device);
  std::vector<const char *> getRequiredExtensions();
  bool checkValidationLayerSupport();
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
  void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
  void hasGflwRequiredInstanceExtensions();
  bool checkDeviceExtensionSupport(VkPhysicalDevice device);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

  VkInstance instance{};
  VkDebugUtilsMessengerEXT debugMessenger{};
  std::vector<VkPhysicalDevice> physicalDevices;
  TpWindow &window;
  VkCommandPool commandPool{};
  VmaAllocator allocator_{};


  VkDevice device_{};
  VkSurfaceKHR surface_{};
  VkQueue graphicsQueue_{};
  VkQueue presentQueue_{};

  const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
  const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  void initializeAllocator();
};

}  // namespace teapot