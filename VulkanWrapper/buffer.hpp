#pragma once
#include "../base.h"
#include "commandBuffer.hpp"
#include "commandPool.hpp"
#include "device.hpp"

namespace VK::Wrapper {
class Buffer {
private:
  VkBuffer mBuffer{VK_NULL_HANDLE};
  VkDeviceMemory mBufferMemory{VK_NULL_HANDLE};
  Device::Ptr mDevice{nullptr};
  VkDescriptorBufferInfo m_BufferInfo{};

public:
  using Ptr = std::shared_ptr<Buffer>;
  static Ptr Create(const Device::Ptr &device, VkDeviceSize size,
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties) {
    return std::make_shared<Buffer>(device, size, usage, properties);
  }
  Buffer(const Device::Ptr &device, VkDeviceSize size, VkBufferUsageFlags usage,
         VkMemoryPropertyFlags properties);

  ~Buffer();

public:
  static Ptr CreateVertexBuffer(const Device::Ptr &device, VkDeviceSize size,
                                void *pData);
  static Ptr CreateIndexBuffer(const Device::Ptr &device, VkDeviceSize size,
                               void *pData);
  static Ptr CreateUniformBuffer(const Device::Ptr &device, VkDeviceSize size,
                                 void *pData = nullptr);
  static Ptr CreateStageBuffer(const Device::Ptr &device, VkDeviceSize size,
                               void *pData = nullptr);

  void UpdateBufferByMap(void *data, size_t size);

  void UpdateBufferByStage(void *data, size_t size);

  void CopyBuffer(const VkBuffer &srcBuffer, const VkBuffer &dstBuffer,
                  VkDeviceSize size);

  [[nodiscard]] auto getBuffer() const { return mBuffer; }
  [[nodiscard]] auto &GetBufferInfo() { return m_BufferInfo; }

private:
  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
};

Buffer::Buffer(const Device::Ptr &device, VkDeviceSize size,
               VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
  mDevice = device;

  VkBufferCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  createInfo.size = size;
  createInfo.usage = usage;
  createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(mDevice->GetDevice(), &createInfo, nullptr, &mBuffer) !=
      VK_SUCCESS) {
    throw std::runtime_error("Error:failed to create buffer");
  }

  VkMemoryRequirements memReq{};
  vkGetBufferMemoryRequirements(mDevice->GetDevice(), mBuffer, &memReq);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memReq.size;
  // memoryTypeBits里存放了所有可用的memoryType
  allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

  if (vkAllocateMemory(mDevice->GetDevice(), &allocInfo, nullptr,
                       &mBufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to allocate memory");
  }

  vkBindBufferMemory(mDevice->GetDevice(), mBuffer, mBufferMemory, 0);
  m_BufferInfo.buffer = mBuffer;
  m_BufferInfo.offset = 0;
  m_BufferInfo.range = size;
}
Buffer::~Buffer() {
  if (mBuffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(mDevice->GetDevice(), mBuffer, nullptr);
  }

  if (mBufferMemory != VK_NULL_HANDLE) {
    vkFreeMemory(mDevice->GetDevice(), mBufferMemory, nullptr);
  }
}
uint32_t Buffer::findMemoryType(uint32_t typeFilter,
                                VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProps;
  vkGetPhysicalDeviceMemoryProperties(mDevice->GetPhysicalDevice(), &memProps);

  for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
    // 该type可用&&该type符合要求
    if ((typeFilter & (1 << i)) &&
        ((memProps.memoryTypes[i].propertyFlags & properties) == properties)) {
      return i;
    }
  }

  throw std::runtime_error("Error: cannot find the property memory type!");
}
// 从cpu端写入到GPU显存
void Buffer::UpdateBufferByMap(void *data, size_t size) {
  void *memPtr = nullptr;

  vkMapMemory(mDevice->GetDevice(), mBufferMemory, 0, size, 0, &memPtr);
  memcpy(memPtr, data, size);
  vkUnmapMemory(mDevice->GetDevice(), mBufferMemory);
}

void Buffer::UpdateBufferByStage(void *data, size_t size) {
  auto stageBuffer =
      Buffer::Create(mDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  stageBuffer->UpdateBufferByMap(data, size);

  CopyBuffer(stageBuffer->getBuffer(), mBuffer,
             static_cast<VkDeviceSize>(size));
}

void Buffer::CopyBuffer(const VkBuffer &srcBuffer, const VkBuffer &dstBuffer,
                        VkDeviceSize size) {
  auto commandPool = CommandPool::Create(mDevice);
  auto commandBuffer = CommandBuffer::Create(mDevice, commandPool);

  commandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  VkBufferCopy copyInfo{};
  copyInfo.size = size;

  commandBuffer->CopyBufferToBuffer(srcBuffer, dstBuffer, 1, {copyInfo});

  commandBuffer->End();

  commandBuffer->SubmitSync(mDevice->GetGraphicQueue());
}
Buffer::Ptr Buffer::CreateVertexBuffer(const Device::Ptr &device,
                                       VkDeviceSize size, void *pData) {
  auto buffer = Buffer::Create(device, size,
                               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  buffer->UpdateBufferByStage(pData, size);

  return buffer;
}
Buffer::Ptr Buffer::CreateIndexBuffer(const Device::Ptr &device,
                                      VkDeviceSize size, void *pData) {
  auto buffer = Buffer::Create(device, size,
                               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  buffer->UpdateBufferByStage(pData, size);

  return buffer;
}

Buffer::Ptr Buffer::CreateUniformBuffer(const Device::Ptr &device,
                                        VkDeviceSize size, void *pData) {
  auto buffer = Buffer::Create(device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if (pData != nullptr) {
    buffer->UpdateBufferByStage(pData, size);
  }

  return buffer;
}

//用于填充某张Image的内容，因此该buffer必是TRANSFER_SRC
Buffer::Ptr Buffer::CreateStageBuffer(const Device::Ptr &device,
                                      VkDeviceSize size, void *pData) {
  auto buffer = Buffer::Create(device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if (pData != nullptr) {
    buffer->UpdateBufferByMap(pData, size);
  }

  return buffer;
}
} // namespace VK::Wrapper