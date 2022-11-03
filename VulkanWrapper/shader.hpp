#pragma once
#include "../base.h"
#include "device.hpp"
#include <fstream>

namespace VK::Wrapper {

class Shader {
private:
  VkShaderModule m_ShaderModule{VK_NULL_HANDLE};
  Device::Ptr m_Device{nullptr};
  std::string m_EntryPoint;
  VkShaderStageFlagBits m_ShaderStage;

public:
  using Ptr = std::shared_ptr<Shader>;
  static Ptr Create(const Device::Ptr &device, const std::string &fileName,
                    VkShaderStageFlagBits shaderStage,
                    const std::string &entryPoint) {
    return std::make_shared<Shader>(device, fileName, shaderStage, entryPoint);
  }

  Shader(const Device::Ptr &device, const std::string &fileName,
         VkShaderStageFlagBits shaderStage, const std::string &entryPoint);

  ~Shader();
  [[nodiscard]] auto GetShaderStage() const { return m_ShaderStage; }
  [[nodiscard]] auto& GetShaderEntryPoint() const { return m_EntryPoint; }
  [[nodiscard]] auto GetShaderModule() const { return m_ShaderModule; }

  auto Make_Createinfo_in_pipeline();
};

static std::vector<char> ReadBinary(const std::string &fileName) {
  std::ifstream file(fileName.c_str(),
                     std::ios::ate | std::ios::binary | std::ios::in);

  if (!file) {
    throw std::runtime_error("Error: failed to open shader file");
  }

  const size_t fileSize = file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}
Shader::Shader(const Device::Ptr &device, const std::string &fileName,
               VkShaderStageFlagBits shaderStage,
               const std::string &entryPoint) {
  m_Device = device;
  m_ShaderStage = shaderStage;
  m_EntryPoint = entryPoint;

  std::vector<char> codeBuffer = ReadBinary(fileName);

  VkShaderModuleCreateInfo shaderCreateInfo{};
  shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  shaderCreateInfo.codeSize = codeBuffer.size();
  shaderCreateInfo.pCode =
      reinterpret_cast<const uint32_t *>(codeBuffer.data());

  if (vkCreateShaderModule(m_Device->GetDevice(), &shaderCreateInfo, nullptr,
                           &m_ShaderModule) != VK_SUCCESS) {
    throw std::runtime_error("Error: failed to create shader");
  }
}
Shader::~Shader() {
  if (m_ShaderModule != VK_NULL_HANDLE) {
    vkDestroyShaderModule(m_Device->GetDevice(), m_ShaderModule, nullptr);
  }
}
auto Shader::Make_Createinfo_in_pipeline() {

  VkPipelineShaderStageCreateInfo shaderCreateInfo{};
  shaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderCreateInfo.stage = m_ShaderStage;
  shaderCreateInfo.pName = m_EntryPoint.c_str();
  shaderCreateInfo.module = m_ShaderModule;
  return shaderCreateInfo;
}

} // namespace VK::Wrapper