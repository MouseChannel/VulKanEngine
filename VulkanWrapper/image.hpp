#pragma once
#include "../base.h"
#include "buffer.hpp"
#include "commandBuffer.hpp"
#include "commandPool.hpp"
#include "device.hpp"
#include "vulkan/vulkan_core.h"

namespace VK::Wrapper {
class Image {
private:
  size_t m_Width{0};
  size_t m_Height{0};
  Device::Ptr m_Device{nullptr};
  VkImage m_Image{VK_NULL_HANDLE};
  VkDeviceMemory m_ImageMemory{VK_NULL_HANDLE};
  VkImageView m_ImageView{VK_NULL_HANDLE};
  VkFormat m_Format;

  VkImageLayout m_Layout{VK_IMAGE_LAYOUT_UNDEFINED};
  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);

public:
  using Ptr = std::shared_ptr<Image>;
  static Ptr Create(const Device::Ptr &device, const int &width,
                    const int &height, const VkFormat &format,
                    const VkImageType &imageType, const VkImageTiling &tiling,
                    const VkImageUsageFlags &usage,
                    const VkSampleCountFlagBits &sample,
                    const VkMemoryPropertyFlags &properties,
                    const VkImageAspectFlags &aspectFlags) {
    return std::make_shared<Image>(device, width, height, format, imageType,
                                   tiling, usage, sample, properties,
                                   aspectFlags);
  }
  Image(const Device::Ptr &device, const int &width, const int &height,
        const VkFormat &format, const VkImageType &imageType,
        const VkImageTiling &tiling, const VkImageUsageFlags &usage,
        const VkSampleCountFlagBits &sample,
        const VkMemoryPropertyFlags &properties,
        const VkImageAspectFlags &aspectFlags);
  ~Image();

  void SetImageLayout(VkImageLayout newLayout,
                      VkPipelineStageFlags srcStageMask,
                      VkPipelineStageFlags dstStageMask,
                      VkImageSubresourceRange subresrouceRange,
                      const CommandPool::Ptr &commandPool);

  void FillImageData(size_t size, void *pData,
                     const CommandPool::Ptr &commandPool);
  uint32_t FindMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
  bool hasStencilComponent(VkFormat format);
  [[nodiscard]] auto GetLayout() { return m_Layout; }
  [[nodiscard]] auto GetImage() { return m_Image; }
  [[nodiscard]] auto GetImageView() { return m_ImageView; }

public:
  static Image::Ptr createDepthImage(const Device::Ptr &device,
                                     const int &width, const int &height,
                                     VkSampleCountFlagBits sample);

  static Image::Ptr createRenderTargetImage(const Device::Ptr &device,
                                            const int &width, const int &height,
                                            VkFormat format);
  static VkFormat findDepthFormat(const Device::Ptr &device);
  static VkFormat findSupportedFormat(const Device::Ptr &device,
                                      const std::vector<VkFormat> &candidates,
                                      VkImageTiling tiling,
                                      VkFormatFeatureFlags features);
};
uint32_t Image::FindMemoryType(uint32_t typeFilter,
                               VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProps;
  vkGetPhysicalDeviceMemoryProperties(m_Device->GetPhysicalDevice(), &memProps);

  for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
    if ((typeFilter & (1 << i)) &&
        ((memProps.memoryTypes[i].propertyFlags & properties) == properties)) {
      return i;
    }
  }

  throw std::runtime_error("Error: cannot find the property memory type!");
}

Image::Image(const Device::Ptr &device, const int &width, const int &height,
             const VkFormat &format, const VkImageType &imageType,
             const VkImageTiling &tiling, const VkImageUsageFlags &usage,
             const VkSampleCountFlagBits &sample,
             const VkMemoryPropertyFlags &properties,
             const VkImageAspectFlags &aspectFlags) {
  m_Device = device;
  m_Layout = VK_IMAGE_LAYOUT_UNDEFINED;
  m_Width = width;
  m_Height = height;
  m_Format = format;

  VkImageCreateInfo imageCreateInfo{};
  imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.extent.width = width;
  imageCreateInfo.extent.height = height;
  imageCreateInfo.extent.depth = 1;
  imageCreateInfo.format = format; // rgb rgba
  imageCreateInfo.imageType = imageType;
  imageCreateInfo.tiling = tiling;
  imageCreateInfo.usage = usage; // color depth?
  imageCreateInfo.samples = sample;
  imageCreateInfo.mipLevels = 1;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(m_Device->GetDevice(), &imageCreateInfo, nullptr,
                    &m_Image) != VK_SUCCESS) {
    throw std::runtime_error("Error:failed to create image");
  }

  VkMemoryRequirements memReq{};
  vkGetImageMemoryRequirements(m_Device->GetDevice(), m_Image, &memReq);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memReq.size;

  allocInfo.memoryTypeIndex = FindMemoryType(memReq.memoryTypeBits, properties);

  if (vkAllocateMemory(m_Device->GetDevice(), &allocInfo, nullptr,
                       &m_ImageMemory) != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to allocate memory");
  }

  vkBindImageMemory(m_Device->GetDevice(), m_Image, m_ImageMemory, 0);

  VkImageViewCreateInfo imageViewCreateInfo{};
  imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  imageViewCreateInfo.viewType = imageType == VK_IMAGE_TYPE_2D
                                     ? VK_IMAGE_VIEW_TYPE_2D
                                     : VK_IMAGE_VIEW_TYPE_3D;
  imageViewCreateInfo.format = format;
  imageViewCreateInfo.image = m_Image;
  imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
  imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
  imageViewCreateInfo.subresourceRange.levelCount = 1;
  imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
  imageViewCreateInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(m_Device->GetDevice(), &imageViewCreateInfo, nullptr,
                        &m_ImageView) != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to create image view");
  }
}
Image::~Image() {
  if (m_ImageView != VK_NULL_HANDLE) {
    vkDestroyImageView(m_Device->GetDevice(), m_ImageView, nullptr);
  }

  if (m_ImageMemory != VK_NULL_HANDLE) {
    vkFreeMemory(m_Device->GetDevice(), m_ImageMemory, nullptr);
  }

  if (m_Image != VK_NULL_HANDLE) {
    vkDestroyImage(m_Device->GetDevice(), m_Image, nullptr);
  }
}

// 使用barrier修改image格式
void Image::SetImageLayout(VkImageLayout newLayout,
                           VkPipelineStageFlags srcStageMask,
                           VkPipelineStageFlags dstStageMask,
                           VkImageSubresourceRange subresrouceRange,
                           const CommandPool::Ptr &commandPool) {

  VkImageMemoryBarrier imageMemoryBarrier{};
  imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageMemoryBarrier.oldLayout = m_Layout;
  imageMemoryBarrier.newLayout = newLayout;
  imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.image = m_Image;
  imageMemoryBarrier.subresourceRange = subresrouceRange;

  switch (m_Layout) {
    // 说明图片刚被创建，上方是初始化的虚拟操作
  case VK_IMAGE_LAYOUT_UNDEFINED:
    imageMemoryBarrier.srcAccessMask = 0;
    break;
    // 如果要写入该图片，需要等待write操作完成
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    break;
  default:
    break;
  }

  switch (newLayout) {
    // 如果目标是：将图片作为被写入的对象，则被阻塞的操作必定是write操作
  case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    break;
    // 如果目标是：将格式转化为一个可读的纹理，那么被阻塞的必定是read操作
    //如果该image作为texture，那么来源只能是：1.通过map从cpu端拷贝过来，2.通过stagingBuffer拷贝而来
  case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
    imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    break;
  default:
    break;
  }

  m_Layout = newLayout;

  auto commandBuffer = Wrapper::CommandBuffer::Create(m_Device, commandPool);
  commandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  commandBuffer->TransferImageLayout(imageMemoryBarrier, srcStageMask,
                                     dstStageMask);
  commandBuffer->End();

  commandBuffer->SubmitSync(m_Device->GetGraphicQueue());
}
// 填充该image内容
void Image::FillImageData(size_t size, void *pData,
                          const CommandPool::Ptr &commandPool) {
  assert(pData);
  assert(size);

  auto stageBuffer = Buffer::CreateStageBuffer(m_Device, size, pData);

  auto commandBuffer = CommandBuffer::Create(m_Device, commandPool);
  commandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  commandBuffer->CopyBufferToImage(stageBuffer->getBuffer(), m_Image, m_Layout,
                                   m_Width, m_Height);
  commandBuffer->End();

  commandBuffer->SubmitSync(m_Device->GetGraphicQueue());
}
Image::Ptr Image::createDepthImage(const Device::Ptr &device, const int &width,
                                   const int &height,
                                   VkSampleCountFlagBits samples) {
  std::vector<VkFormat> formats = {
      VK_FORMAT_D32_SFLOAT,
      VK_FORMAT_D32_SFLOAT_S8_UINT,
      VK_FORMAT_D24_UNORM_S8_UINT,
  };

  VkFormat resultFormat =
      findSupportedFormat(device, formats, VK_IMAGE_TILING_OPTIMAL,
                          VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

  return Image::Create(
      device, width, height, resultFormat, VK_IMAGE_TYPE_2D,
      VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      samples, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
}
Image::Ptr Image::createRenderTargetImage(const Device::Ptr &device,
                                          const int &width, const int &height,
                                          VkFormat format) {
  return Image::Create(

      device, width, height, format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,

      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, device->getMaxUsableSampleCount(),

      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,

      VK_IMAGE_ASPECT_COLOR_BIT);
}

VkFormat Image::findDepthFormat(const Device::Ptr &device) {
  std::vector<VkFormat> formats = {
      VK_FORMAT_D32_SFLOAT,
      VK_FORMAT_D32_SFLOAT_S8_UINT,
      VK_FORMAT_D24_UNORM_S8_UINT,
  };

  return findSupportedFormat(device, formats, VK_IMAGE_TILING_OPTIMAL,
                             VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
VkFormat Image::findSupportedFormat(const Device::Ptr &device,
                                    const std::vector<VkFormat> &candidates,
                                    VkImageTiling tiling,
                                    VkFormatFeatureFlags features) {
  for (auto format : candidates) {
    VkFormatProperties props;

    vkGetPhysicalDeviceFormatProperties(device->GetPhysicalDevice(), format,
                                        &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    }

    if (tiling == VK_IMAGE_TILING_OPTIMAL &&
        (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("Error: can not find proper format");
}
bool Image::hasStencilComponent(VkFormat format) {
  return m_Format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         m_Format == VK_FORMAT_D24_UNORM_S8_UINT;
}

} // namespace VK::Wrapper
