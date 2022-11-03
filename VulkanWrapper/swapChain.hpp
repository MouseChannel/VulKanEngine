#include "../base.h"
#include "device.hpp"
#include "renderPass.hpp"
#include "vulkan/vulkan_core.h"

namespace VK::Wrapper {
struct SwapChainSupportInfo {
  // 基础表面特性(交换链的最小/最大图像数量，最小/最大图像宽度、高度)
  VkSurfaceCapabilitiesKHR m_Capabilities;
  // 表面格式(像素格式，颜色空间)
  std::vector<VkSurfaceFormatKHR> m_Formats;
  // 可用的呈现模式
  std::vector<VkPresentModeKHR> m_PresentModes;
};

class SwapChain {
private:
  VkSwapchainKHR m_SwapChain{VK_NULL_HANDLE};

  VkFormat m_SwapChainFormat;
  VkExtent2D m_SwapChainExtent;

  uint32_t imageCount{0};
  // 交换链中图像句柄
  std::vector<VkImage> m_SwapChainImages{};
  // image管理器
  std::vector<VkImageView> m_SwapChainImageViews{};
  std::vector<VkFramebuffer> m_SwapChainFrameBuffers{};

  Device::Ptr m_Device{nullptr};
  Window::Ptr m_Window{nullptr};
  WindowSurface::Ptr m_Surface{nullptr};
  VkImageView CreateImageView(VkImage image, VkFormat format,
                              VkImageAspectFlags aspectFlags,
                              uint32_t mipLevels = 1);

public:
  using Ptr = std::shared_ptr<SwapChain>;
  static Ptr Create(const Device::Ptr &device, const Window::Ptr &window,
                    const WindowSurface::Ptr &surface) {
    return std::make_shared<SwapChain>(device, window, surface);
  }

  SwapChain(const Device::Ptr &device, const Window::Ptr &window,
            const WindowSurface::Ptr &surface);
  ~SwapChain();
  void CreateFrameBuffers(const RenderPass::Ptr &renderPass);
  // 查看设备支持的格式
  SwapChainSupportInfo QuerySwapChainSupportInfo();

  VkSurfaceFormatKHR
  ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

  VkPresentModeKHR ChooseSurfacePresentMode(
      const std::vector<VkPresentModeKHR> &availablePresenstModes);

  VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR &capabilities);
  [[nodiscard]] auto GetFormat() const { return m_SwapChainFormat; }
  [[nodiscard]] auto GetImageCount() const { return imageCount; }
  [[nodiscard]] auto GetFrameBuffer(const int index) const {
    return m_SwapChainFrameBuffers[index];
  }
  [[nodiscard]] auto GetExtent() const { return m_SwapChainExtent; }
  [[nodiscard]] auto GetSwapChain() { return m_SwapChain; }
};

SwapChain::SwapChain(const Device::Ptr &device, const Window::Ptr &window,
                     const WindowSurface::Ptr &surface) {
  m_Device = device;
  m_Window = window;
  m_Surface = surface;
  auto swapChainInfo = QuerySwapChainSupportInfo();
  auto surfaceFormat = ChooseSurfaceFormat(swapChainInfo.m_Formats);
  auto presentMode = ChooseSurfacePresentMode(swapChainInfo.m_PresentModes);
  auto extent = ChooseExtent(swapChainInfo.m_Capabilities);
  // 设置交换链中图像的个数
  imageCount = swapChainInfo.m_Capabilities.minImageCount + 1;
  if (swapChainInfo.m_Capabilities.maxImageCount > 0 &&
      imageCount > swapChainInfo.m_Capabilities.maxImageCount) {
    imageCount = swapChainInfo.m_Capabilities.maxImageCount;
  }
  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = m_Surface->GetSurface();
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;

  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  std::vector<uint32_t> queueFamilies = {
      m_Device->GetGraphicQueueFamily().value(),
      m_Device->GetPresentQueueFamily().value()};

  if (m_Device->GetGraphicQueueFamily().value() ==
      m_Device->GetPresentQueueFamily().value()) {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount =
        static_cast<uint32_t>(queueFamilies.size());
    createInfo.pQueueFamilyIndices = queueFamilies.data();
  }
  // 位置不做 任何变换操作
  createInfo.preTransform = swapChainInfo.m_Capabilities.currentTransform;
  // 忽略alpha通道
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  //
  createInfo.presentMode = presentMode;
  // VK_TRUE表示我们不关心被窗口系统中的其它窗口遮挡的像素的颜色
  createInfo.clipped = VK_TRUE;
  // 此次还没有任何老交换链，置空即可
  createInfo.oldSwapchain = VK_NULL_HANDLE;
  if (vkCreateSwapchainKHR(m_Device->GetDevice(), &createInfo, nullptr,
                           &m_SwapChain) != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to create swapChain");
  }

  m_SwapChainFormat = surfaceFormat.format;
  m_SwapChainExtent = extent;

  // 获取交换链图像句柄
  vkGetSwapchainImagesKHR(device->GetDevice(), m_SwapChain, &imageCount,
                          nullptr);
  if (imageCount > 0) {

    m_SwapChainImages.resize(imageCount);
    auto a = vkGetSwapchainImagesKHR(device->GetDevice(), m_SwapChain,
                                     &imageCount, m_SwapChainImages.data());
    int r = m_SwapChainImages.size();
    for (auto &i : m_SwapChainImages) {
      int e = 0;
      int ee = 0;
    }
  }
  // 生成图像管理器
  m_SwapChainImageViews.resize(imageCount);
  for (int i = 0; i < imageCount; ++i) {
    m_SwapChainImageViews[i] = CreateImageView(
        m_SwapChainImages[i], m_SwapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
  }
  // 创建framebuffer
}
VkImageView SwapChain::CreateImageView(VkImage image, VkFormat format,
                                       VkImageAspectFlags aspectFlags,
                                       uint32_t mipLevels) {
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;

  viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView{VK_NULL_HANDLE};
  if (vkCreateImageView(m_Device->GetDevice(), &viewInfo, nullptr,
                        &imageView) != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to create image view in swapchain");
  }

  return imageView;
}

void SwapChain::CreateFrameBuffers(const RenderPass::Ptr &renderPass) {

  m_SwapChainFrameBuffers.resize(imageCount);
  for (int i = 0; i < imageCount; ++i) {
    // framebuffer 包括一帧所有的数据
    // 包括多个 colorAttachMent 和一个 DepthStencilAttachMent
    std::array<VkImageView, 1> attachments = {m_SwapChainImageViews[i]};

    VkFramebufferCreateInfo frameBufferCreateInfo{};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.renderPass = renderPass->GetRenderPass();
    frameBufferCreateInfo.attachmentCount =
        static_cast<uint32_t>(attachments.size());
    frameBufferCreateInfo.pAttachments = attachments.data();
    frameBufferCreateInfo.width = m_SwapChainExtent.width;
    frameBufferCreateInfo.height = m_SwapChainExtent.height;
    frameBufferCreateInfo.layers = 1;

    if (vkCreateFramebuffer(m_Device->GetDevice(), &frameBufferCreateInfo,
                            nullptr,
                            &m_SwapChainFrameBuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("Error:Failed to create frameBuffer");
    }
  }
}
SwapChain::~SwapChain() {
  vkDestroySwapchainKHR(m_Device->GetDevice(), m_SwapChain, nullptr);
  for (auto imageView : m_SwapChainImageViews) {
    vkDestroyImageView(m_Device->GetDevice(), imageView, nullptr);
  }
  for (auto &i : m_SwapChainFrameBuffers) {
    vkDestroyFramebuffer(m_Device->GetDevice(), i, nullptr);
  }
  m_Window.reset();
  m_Surface.reset();
  m_Device.reset();
}
SwapChainSupportInfo SwapChain::QuerySwapChainSupportInfo() {
  SwapChainSupportInfo info{};
  // 获取基础特性
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device->GetPhysicalDevice(),
                                            m_Surface->GetSurface(),
                                            &info.m_Capabilities);

  uint32_t formatCount = 0;
  // 获取表面支持格式
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetPhysicalDevice(),
                                       m_Surface->GetSurface(), &formatCount,
                                       nullptr);

  if (formatCount != 0) {
    info.m_Formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetPhysicalDevice(),
                                         m_Surface->GetSurface(), &formatCount,
                                         info.m_Formats.data());
  }

  uint32_t presentModeCount = 0;
  // 获取呈现模式
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_Device->GetPhysicalDevice(),
                                            m_Surface->GetSurface(),
                                            &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    info.m_PresentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_Device->GetPhysicalDevice(), m_Surface->GetSurface(),
        &presentModeCount, info.m_PresentModes.data());
  }

  return info;
}
VkSurfaceFormatKHR SwapChain::ChooseSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {

  if (availableFormats.size() == 1 &&
      availableFormats[0].format == VK_FORMAT_UNDEFINED) {
    return {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
  }

  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}
VkPresentModeKHR SwapChain::ChooseSurfacePresentMode(
    const std::vector<VkPresentModeKHR> &availablePresenstModes) {

  VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

  for (const auto &availablePresentMode : availablePresenstModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    } else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      bestMode = availablePresentMode;
    }
  }

  return bestMode;
}
// 交换链中图像的分辨率，一般与窗口的分辨率相同
VkExtent2D
SwapChain::ChooseExtent(const VkSurfaceCapabilitiesKHR &capabilities) {

  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  int width = 0, height = 0;
  glfwGetFramebufferSize(m_Window->GetWindow(), &width, &height);

  VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                             static_cast<uint32_t>(height)};
  // VkExtent2D actualExtent = {800, 600};

  actualExtent.width =
      std::max(capabilities.minImageExtent.width,
               std::min(capabilities.maxImageExtent.width, actualExtent.width));
  actualExtent.height = std::max(
      capabilities.minImageExtent.height,
      std::min(capabilities.maxImageExtent.height, actualExtent.height));

  return actualExtent;
}

} // namespace VK::Wrapper