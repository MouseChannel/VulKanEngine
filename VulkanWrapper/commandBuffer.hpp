#pragma once
#include "../base.h"
#include "commandPool.hpp"
#include "device.hpp"
#include "vulkan/vulkan_core.h"


namespace VK::Wrapper {
class CommandBuffer {
private:
  VkCommandBuffer mCommandBuffer{VK_NULL_HANDLE};
  Device::Ptr mDevice{nullptr};
  CommandPool::Ptr mCommandPool{nullptr};

public:
  using Ptr = std::shared_ptr<CommandBuffer>;
  static Ptr Create(const Device::Ptr &device,
                    const CommandPool::Ptr &commandPool,
                    bool asSecondary = false) {
    return std::make_shared<CommandBuffer>(device, commandPool, asSecondary);
  }
  CommandBuffer(const Device::Ptr &device, const CommandPool::Ptr &commandPool,
                bool asSecondary = false);
  ~CommandBuffer();
  [[nodiscard]] auto GetCommandBuffer() const { return mCommandBuffer; }
  void Begin(VkCommandBufferUsageFlags flag = 0,
             const VkCommandBufferInheritanceInfo &inheritance = {}) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = flag;
    beginInfo.pInheritanceInfo = &inheritance;

    if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS) {
      throw std::runtime_error("Error:failed to begin commandBuffer");
    }
  }

  void BeginRenderPass(
      const VkRenderPassBeginInfo &renderPassBeginInfo,
      const VkSubpassContents &subPassContents = VK_SUBPASS_CONTENTS_INLINE) {
    vkCmdBeginRenderPass(mCommandBuffer, &renderPassBeginInfo, subPassContents);
  }

  void BindGraphicPipeline(const VkPipeline &pipeline) {
    vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline);
  }
  void BindDescriptorSet(const VkPipelineLayout layout,
                         const VkDescriptorSet &descriptorSet) {
    vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            layout, 0, 1, &descriptorSet, 0, nullptr);
  }
  void BindVertexBuffer(const std::vector<VkBuffer> &buffers) {
    std::vector<VkDeviceSize> offsets(buffers.size(), 0);

    vkCmdBindVertexBuffers(mCommandBuffer, 0,
                           static_cast<uint32_t>(buffers.size()),
                           buffers.data(), offsets.data());
  }
  void BindIndexBuffer(const VkBuffer &buffer) {
    vkCmdBindIndexBuffer(mCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
  }

  void Draw(size_t vertexCount) {
    vkCmdDraw(mCommandBuffer, vertexCount, 1, 0, 0);
  }

  void DrawIndex(size_t indexCount) {
    vkCmdDrawIndexed(mCommandBuffer, indexCount, 1, 0, 0, 0);
  }

  void EndRenderPass() { vkCmdEndRenderPass(mCommandBuffer); }

  void End() {
    if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS) {
      throw std::runtime_error("Error:failed to end Command Buffer");
    }
  }
  void CopyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                          uint32_t copyInfoCount,
                          const std::vector<VkBufferCopy> &copyInfos) {
    vkCmdCopyBuffer(mCommandBuffer, srcBuffer, dstBuffer, copyInfoCount,
                    copyInfos.data());
  }
  void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage,
                         VkImageLayout dstImageLayout, uint32_t width,
                         uint32_t height) {
    VkBufferImageCopy region{};
    
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(mCommandBuffer, srcBuffer, dstImage, dstImageLayout,
                           1, &region);
  }

  void SubmitSync(VkQueue queue, VkFence fence = VK_NULL_HANDLE);
  void TransferImageLayout(VkImageMemoryBarrier &imageMemoryBarrier,
                           VkPipelineStageFlags srcStageMask,
                           VkPipelineStageFlags dstStageMask) {
    vkCmdPipelineBarrier(mCommandBuffer, srcStageMask, dstStageMask, 0, 0,
                         nullptr,    // MemoryBarrier
                         0, nullptr, // BufferMemoryBarrier
                         1, &imageMemoryBarrier);
  }
};
CommandBuffer::CommandBuffer(const Device::Ptr &device,
                             const CommandPool::Ptr &commandPool,
                             bool asSecondary) {
  mDevice = device;
  mCommandPool = commandPool;

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandBufferCount = 1;
  allocInfo.commandPool = mCommandPool->GetCommandPool();
  allocInfo.level = asSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY
                                : VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  if (vkAllocateCommandBuffers(mDevice->GetDevice(), &allocInfo,
                               &mCommandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("Error: falied to create commandBuffer");
  }
}
CommandBuffer::~CommandBuffer() {
  if (mCommandBuffer != VK_NULL_HANDLE) {
    vkFreeCommandBuffers(mDevice->GetDevice(), mCommandPool->GetCommandPool(),
                         1, &mCommandBuffer);
  }
}
// 等待commandBuffer队列里的所有command全完成
void CommandBuffer::SubmitSync(VkQueue queue, VkFence fence) {
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &mCommandBuffer;

  vkQueueSubmit(queue, 1, &submitInfo, fence);

  vkQueueWaitIdle(queue);
}
} // namespace VK::Wrapper