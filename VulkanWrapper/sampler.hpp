#pragma once
#include"../base.h"
#include "device.hpp"

namespace VK::Wrapper {
	class Sampler {
	private:
		Device::Ptr mDevice{ nullptr };
		VkSampler mSampler{ VK_NULL_HANDLE };
	public:
		using Ptr = std::shared_ptr<Sampler>;
		static Ptr create(const Device::Ptr& device) { return std::make_shared<Sampler>(device); }

		Sampler(const Device::Ptr& device);

		~Sampler();

		[[nodiscard]] auto getSampler() const { return mSampler; }

	};
	Sampler::Sampler(const Device::Ptr& device) {
		mDevice = device;

		VkSamplerCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.magFilter = VK_FILTER_LINEAR;
		createInfo.minFilter = VK_FILTER_LINEAR;
		createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		createInfo.anisotropyEnable = VK_TRUE;
		createInfo.maxAnisotropy = 16;

		createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		//�����Ƿ�ʹ�÷ǹ�һ����uvw������ϵ0-1
		createInfo.unnormalizedCoordinates = VK_FALSE;

		//�����õ�������ֵ��Ҫ��һ��ֵ���бȽϣ��Ƚϵķ�ʽҲ�������ã��Ƚ�ͨ���˲��ԣ��Ų��������򲻲���
		createInfo.compareEnable = VK_FALSE;
		createInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		createInfo.mipLodBias = 0.0f;
		createInfo.minLod = 0.0f;
		createInfo.maxLod = 0.0f;

		if (vkCreateSampler(mDevice->GetDevice(), &createInfo, nullptr, &mSampler) != VK_SUCCESS) {
			throw std::runtime_error("Error: failed to create sampler");
		}
	}

	Sampler::~Sampler() {
		if (mSampler != VK_NULL_HANDLE) {
			vkDestroySampler(mDevice->GetDevice(), mSampler, nullptr);
		}
	}




}