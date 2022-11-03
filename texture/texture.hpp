#pragma once
#include "../base.h"
#include "../VulkanWrapper/device.hpp"
#include "../VulkanWrapper/commandPool.hpp"
#include "../VulkanWrapper/image.hpp"
#include "../VulkanWrapper/sampler.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"
namespace VK {

	class Texture {
	private:
		Wrapper::Device::Ptr mDevice{ nullptr };
		Wrapper::Image::Ptr mImage{ nullptr };
		Wrapper::Sampler::Ptr mSampler{ nullptr };
		VkDescriptorImageInfo mImageInfo{};

	public:
		using Ptr = std::shared_ptr<Texture>;
		static Ptr create(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath) {
			return std::make_shared<Texture>(device, commandPool, imageFilePath);
		}

		Texture(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath);

		~Texture() ;
		[[nodiscard]] auto& GetImageInfo(){
			return mImageInfo;
		}



	};
	Texture::Texture(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath)
	{

		mDevice = device;

		int texWidth, texHeight, texSize, texChannles;
		stbi_uc* pixels = stbi_load(imageFilePath.c_str(), &texWidth, &texHeight, &texChannles, STBI_rgb_alpha);
		std::cout<<imageFilePath.c_str()<<std::endl;
		if (!pixels) {
			throw std::runtime_error("Error: failed to read image data");
		}

		texSize = texWidth * texHeight * 4;

		mImage = Wrapper::Image::Create(
			mDevice, texWidth, texHeight,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		VkImageSubresourceRange region{};
		region.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		region.baseArrayLayer = 0;
		region.layerCount = 1;

		region.baseMipLevel = 0;
		region.levelCount = 1;

		mImage->SetImageLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			region,
			commandPool
		);

		mImage->FillImageData(texSize, (void*)pixels, commandPool);

		mImage->SetImageLayout(
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			region,
			commandPool
		);

		stbi_image_free(pixels);

		mSampler = Wrapper::Sampler::create(mDevice);

		mImageInfo.imageLayout = mImage->GetLayout();
		mImageInfo.imageView = mImage->GetImageView();
		mImageInfo.sampler = mSampler->getSampler();

	}
	Texture::~Texture() {

	}



}