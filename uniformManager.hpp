#pragma once
#include "VulkanWrapper/description.h"
#include "VulkanWrapper/descriptorPool.hpp"
#include "VulkanWrapper/descriptorSet.hpp"
#include "VulkanWrapper/descriptorSetLayout.hpp"
#include "VulkanWrapper/device.hpp"
#include "base.h"

namespace VK::Wrapper {

	class UniformManager {
	private:
		std::vector<Wrapper::UniformParameter::Ptr> m_UniformParams;

		Wrapper::DescriptorSetLayout::Ptr m_DescriptorSetLayout{ nullptr };
		Wrapper::DescriptorPool::Ptr m_DescriptorPool{ nullptr };
		Wrapper::DescriptorSet::Ptr m_DescriptorSet{ nullptr };
		Wrapper::Device::Ptr m_Device{ nullptr };

	public:
		using Ptr = std::shared_ptr<UniformManager>;
		static Ptr Create() { return std::make_shared<UniformManager>(); }

		UniformManager() = default;

		~UniformManager() = default;
		void Init(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, int frameCount);

		void Update(const VPMatrices& vpMatrices, const ObjectUniform& objectUniform,
			const int& frameCount);
		[[nodiscard]] auto& GetDescriptorLayout() const {
			return m_DescriptorSetLayout;
		}

		[[nodiscard]] auto GetDescriptorSet(int frameCount) const {
			return m_DescriptorSet->GetDescriptorSet(frameCount);
		}
	};

	void UniformManager::Init(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, int frameCount) {
		m_Device = device;
		auto vpParam = Wrapper::UniformParameter::create();
		vpParam->mBinding = 0;
		vpParam->mCount = 1;
		vpParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vpParam->mSize = sizeof(VPMatrices);
		vpParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;

		for (int i = 0; i < frameCount; ++i) {
			auto buffer =
				Wrapper::Buffer::CreateUniformBuffer(device, vpParam->mSize, nullptr);
			vpParam->m_Buffers.push_back(buffer);
		}

		m_UniformParams.push_back(vpParam);

		auto objectParam = Wrapper::UniformParameter::create();
		objectParam->mBinding = 1;
		objectParam->mCount = 1;
		objectParam->mDescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		objectParam->mSize = sizeof(ObjectUniform);
		objectParam->mStage = VK_SHADER_STAGE_VERTEX_BIT;

		for (int i = 0; i < frameCount; ++i) {
			auto buffer = Wrapper::Buffer::CreateUniformBuffer(
				device, objectParam->mSize, nullptr);
			objectParam->m_Buffers.push_back(buffer);
		}

		m_UniformParams.push_back(objectParam);

		auto textureParam = Wrapper::UniformParameter::create();
		textureParam->mBinding = 2;
		textureParam->mCount = 1;
		textureParam->mDescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureParam->mStage = VK_SHADER_STAGE_FRAGMENT_BIT;
		textureParam->mTexture = Texture::create(m_Device, commandPool, "./assets/dragonBall.jpg");

		m_UniformParams.push_back(textureParam);



		m_DescriptorSetLayout = Wrapper::DescriptorSetLayout::Create(device);
		m_DescriptorSetLayout->Build(m_UniformParams);


		m_DescriptorPool = Wrapper::DescriptorPool::Create(device);
		m_DescriptorPool->Build(m_UniformParams, frameCount);

		m_DescriptorSet = Wrapper::DescriptorSet::Create(
			device, m_UniformParams, m_DescriptorSetLayout, m_DescriptorPool,
			frameCount);
	}

	void UniformManager::Update(const VPMatrices& vpMatrices,
		const ObjectUniform& objectUniform,
		const int& frameCount) {
		m_UniformParams[0]->m_Buffers[frameCount]->UpdateBufferByMap(
			(void*)(&vpMatrices), sizeof(VPMatrices));

		// update object uniform
		m_UniformParams[1]->m_Buffers[frameCount]->UpdateBufferByMap(
			(void*)(&objectUniform), sizeof(ObjectUniform));
	}
} // namespace VK::Wrapper