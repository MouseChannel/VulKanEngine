#pragma once

#include "VulkanWrapper/buffer.hpp"
#include "VulkanWrapper/device.hpp"
#include "base.h"
#include "vulkan/vulkan_core.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace VK {
struct Vertex {
  glm::vec3 mPosition;
  glm::vec3 mColor;
};
class Model {
private:
  std::vector<float> mPositions{};
  std::vector<float> mColors{};
  std::vector<unsigned int> mIndexDatas{};
  std::vector<float> mUVs{};
  Wrapper::Buffer::Ptr mPositionBuffer{nullptr};
  Wrapper::Buffer::Ptr mColorBuffer{nullptr};
  Wrapper::Buffer::Ptr mUVBuffer{nullptr};

  Wrapper::Buffer::Ptr mIndexBuffer{nullptr};
  ObjectUniform m_Uniform;
  float mAngle{0.0f};

public:
  using Ptr = std::shared_ptr<Model>;
  static Ptr Create(const Wrapper::Device::Ptr &device) {
    return std::make_shared<Model>(device);
  }
  Model(const Wrapper::Device::Ptr &device) {
    m_Uniform.mModelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f),
                                         glm::vec3(1.0f, 0.0f, 0.0f));

    return;
    // mPositions = {0.0f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f };

    // mColors = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

    // mIndexDatas = {0, 1, 2};

    mPositions = {0.0f,  0.5f, 0.0f, 0.5f, 0.0f,  0.0f,
                  -0.5f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f,

                  0.3f,  0.5f, 0.2f, 1.8f, 0.0f,  0.2f,
                  -0.8f, 0.0f, 0.2f, 0.3f, -0.5f, 0.2f};

    mColors = {
        1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f};

    mUVs = {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f};

    mIndexDatas = {0, 2, 1, 1, 2, 3, 4, 6, 5, 5, 6, 7};

    mPositionBuffer = Wrapper::Buffer::CreateVertexBuffer(
        device, mPositions.size() * sizeof(float), mPositions.data());

    mColorBuffer = Wrapper::Buffer::CreateVertexBuffer(
        device, mColors.size() * sizeof(float), mColors.data());
    mUVBuffer = Wrapper::Buffer::CreateVertexBuffer(
        device, mUVs.size() * sizeof(float), mUVs.data());

    mIndexBuffer = Wrapper::Buffer::CreateIndexBuffer(
        device, mIndexDatas.size() * sizeof(float), mIndexDatas.data());
  }
  ~Model() = default;
  std::vector<VkVertexInputBindingDescription>
  getVertexInputBindingDescriptions() {
    std::vector<VkVertexInputBindingDescription> bindingDes{};
    /*bindingDes.resize(1);
    bindingDes[0].binding = 0;
    bindingDes[0].stride = sizeof(Vertex);
    bindingDes[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;*/

    bindingDes.resize(2);

    bindingDes[0].binding = 0;
    bindingDes[0].stride = sizeof(float) * 3;
    bindingDes[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    //
    bindingDes[1].binding = 1;
    bindingDes[1].stride = sizeof(float) * 2;
    bindingDes[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDes;

    //

    bindingDes[1].binding = 1;
    bindingDes[1].stride = sizeof(float) * 3;
    bindingDes[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    bindingDes[2].binding = 2;
    bindingDes[2].stride = sizeof(float) * 2;
    bindingDes[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDes;
  }
  std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDes{};
    attributeDes.resize(2);

    attributeDes[0].binding = 0;
    attributeDes[0].location = 0;
    attributeDes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    // attributeDes[0].offset = offsetof(Vertex, mPosition);
    attributeDes[0].offset = 0;
    //
    attributeDes[1].binding = 1;
    attributeDes[1].location = 2;
    attributeDes[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDes[1].offset = 0;
    return attributeDes;
    //
    // attributeDes[1].binding = 0;
    attributeDes[1].binding = 1;
    attributeDes[1].location = 1;
    attributeDes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    // attributeDes[1].offset = offsetof(Vertex, mColor);
    attributeDes[1].offset = 0;

    attributeDes[2].binding = 2;
    attributeDes[2].location = 2;
    attributeDes[2].format = VK_FORMAT_R32G32_SFLOAT;
    // attributeDes[0].offset = offsetof(Vertex, mPosition);
    attributeDes[2].offset = 0;

    return attributeDes;
  }
  [[nodiscard]] auto getVertexBuffers() const {
    std::vector<VkBuffer> buffers{mPositionBuffer->getBuffer(),
                                  // mColorBuffer->getBuffer(),
                                  mUVBuffer->getBuffer()};

    // std::cout<<a<<std::endl;
    return buffers;
  }

  [[nodiscard]] auto getIndexBuffer() const { return mIndexBuffer; }

  [[nodiscard]] auto getIndexCount() const { return mIndexDatas.size(); }
  [[nodiscard]] auto getUniform() const { return m_Uniform; }

  void setModelMatrix(const glm::mat4 matrix) {
    m_Uniform.mModelMatrix = matrix;
  }
  void update() {
    glm::mat4 rotateMatrix = glm::mat4(1.0f);
    rotateMatrix = glm::rotate(rotateMatrix, glm::radians(mAngle),
                               glm::vec3(0.0f, 0.0f, 1.0f));
    m_Uniform.mModelMatrix = rotateMatrix;

    mAngle += 0.05f;
  }
  void loadModel(const std::string &path, const Wrapper::Device::Ptr &device) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    std::string warn;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                          path.c_str())) {
      throw std::runtime_error("Error: failed to load model");
    }

    for (const auto &shape : shapes) {
      for (const auto &index : shape.mesh.indices) {
        // ��ȡ������λ������
        mPositions.push_back(attrib.vertices[3 * index.vertex_index + 0]);
        mPositions.push_back(attrib.vertices[3 * index.vertex_index + 1]);
        mPositions.push_back(attrib.vertices[3 * index.vertex_index + 2]);

        // ȡ��uvֵ
        mUVs.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
        mUVs.push_back(1.0f - attrib.texcoords[2 * index.texcoord_index + 1]);

        // ����
        mIndexDatas.push_back(mIndexDatas.size());
      }
    }
    std::cout << mPositions.size() << std::endl;

    mPositionBuffer = Wrapper::Buffer::CreateVertexBuffer(
        device, mPositions.size() * sizeof(float), mPositions.data());

    mUVBuffer = Wrapper::Buffer::CreateVertexBuffer(
        device, mUVs.size() * sizeof(float), mUVs.data());

    mIndexBuffer = Wrapper::Buffer::CreateIndexBuffer(
        device, mIndexDatas.size() * sizeof(float), mIndexDatas.data());
  }
};
} // namespace VK