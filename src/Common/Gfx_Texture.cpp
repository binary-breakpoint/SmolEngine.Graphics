#include "Gfx_Precompiled.h"
#include "Common/Gfx_Texture.h"
#include "Backend/Gfx_VulkanHelpers.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <imgui/examples/imgui_impl_vulkan.h>

namespace Dia
{
	bool TextureCreateDesc::Save(const std::string& filePath)
	{
		std::stringstream storage;
		{
			cereal::JSONOutputArchive output{ storage };
			serialize(output);
		}

		std::ofstream myfile(filePath);
		if (myfile.is_open())
		{
			myfile << storage.str();
			myfile.close();
			return true;
		}

		return false;
	}

	bool TextureCreateDesc::Load(const std::string& filePath)
	{
		std::stringstream storage;
		std::ifstream file(filePath);
		if (!file)
		{
			Gfx_Log::LogError("Could not open the file: {}", filePath);
			return false;
		}

		storage << file.rdbuf();
		{
			cereal::JSONInputArchive input{ storage };
			input(myWidth, myHeight, myMipLevels, myArrayLayers, myFormat, myFilePath, myImGUIHandleEnable, myUsage);
		}

		return true;
	}

	Gfx_Texture::Gfx_Texture()
		:
		m_ImguiHandle{nullptr} {}

	Gfx_Texture::~Gfx_Texture()
	{
		Free();
	}

	void Gfx_Texture::Create(TextureCreateDesc* info)
	{
		assert(info != nullptr);

		if (!info->myFilePath.empty())
		{
			int height, width, channels;

			stbi_set_flip_vertically_on_load(1);

			stbi_uc* data = nullptr;
			{
				data = stbi_load(info->myFilePath.c_str(), &width, &height, &channels, 4);
				if (!data)
					RUNTIME_ERROR("VulkanTexture:: Texture not found! file: {}", info->myFilePath);

				info->myWidth = width;
				info->myWidth = height;

				LoadEX(info, data);
			}

			stbi_image_free(data);
			return;
		}

		LoadEX(info, info->myUserData);
	}

	void Gfx_Texture::Free()
	{
		m_PixelStorage.Free();

		for (auto& [key, view] : m_ImageViewMap)
		{
			VK_DESTROY_DEVICE_HANDLE(view, vkDestroyImageView);
		}
	}

	const VkDescriptorImageInfo& Gfx_Texture::GetDescriptorImageInfo() const
	{
		return m_DescriptorImageInfo;
	}

	std::pair<uint32_t, uint32_t> Gfx_Texture::GetMipSize(uint32_t mip) const
	{
		uint32_t width = m_Desc.myWidth;
		uint32_t height = m_Desc.myHeight;
		while (mip != 0)
		{
			width /= 2;
			height /= 2;
			mip--;
		}

		return { width, height };
	}

	VkDescriptorImageInfo Gfx_Texture::GetMipImageView(uint32_t mip)
	{
		if (m_ImageViewMap.find(mip) == m_ImageViewMap.end())
		{
			VkImageViewCreateInfo imageViewCreateInfo = {};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = Gfx_VulkanHelpers::GetFormat(m_Desc.myFormat);
			imageViewCreateInfo.flags = 0;
			imageViewCreateInfo.subresourceRange = {};
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.subresourceRange.baseMipLevel = mip;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;
			imageViewCreateInfo.image = m_PixelStorage.GetImage();

			VkImageView pView = nullptr;
			VK_CHECK_RESULT(vkCreateImageView(Gfx_Context::GetDevice().GetLogicalDevice(), &imageViewCreateInfo, nullptr, &pView));
			m_ImageViewMap[mip] = pView;
		}

		VkDescriptorImageInfo imageinfo{};
		imageinfo.imageLayout = m_DescriptorImageInfo.imageLayout;
		imageinfo.sampler = m_DescriptorImageInfo.sampler;
		imageinfo.imageView = m_ImageViewMap[mip];
		return imageinfo;
	}

	Gfx_PixelStorage* Gfx_Texture::GetPixelStorage()
	{
		return &m_PixelStorage;
	}

	TextureUsage Gfx_Texture::GetUsageFlags() const
	{
		return m_Desc.myUsage;
	}

	void* Gfx_Texture::GetImGuiTexture() const
	{
		return m_ImguiHandle;
	}

	bool Gfx_Texture::IsGood() const
	{
		return m_Desc.myWidth > 0;
	}

	uint32_t Gfx_Texture::GetMips() const
	{
		return m_Desc.myMipLevels;
	}

	void Gfx_Texture::LoadEX(TextureCreateDesc* info, void* data)
	{
		PixelStorageCreateDesc pixelDesc{};
		pixelDesc.myLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		pixelDesc.myArrayLayers = info->myArrayLayers;
		pixelDesc.myFormat = info->myFormat;
		pixelDesc.myWidth = info->myWidth;
		pixelDesc.myHeight = info->myHeight;
		pixelDesc.myMipLevels = info->myMipLevels;
		pixelDesc.myUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		if (info->myUsage == TextureUsage::IMAGE_2D || info->myUsage == TextureUsage::IMAGE_2D_ARRAY)
		{
			pixelDesc.myLayout = VK_IMAGE_LAYOUT_GENERAL;
			pixelDesc.myUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		}

		m_PixelStorage.Create(&pixelDesc);

		if (data != nullptr)
			Gfx_VulkanHelpers::CopyDataToImage(&m_PixelStorage, data);
		else
			Gfx_VulkanHelpers::InitializeImageResource(&m_PixelStorage, m_PixelStorage.GetImageLayout());

		m_DescriptorImageInfo = {};
		m_DescriptorImageInfo.imageLayout = m_PixelStorage.GetImageLayout();
		m_DescriptorImageInfo.imageView = m_PixelStorage.GetImageView();
		m_DescriptorImageInfo.sampler = info->mySampler->GetSampler();

		if(info->myImGUIHandleEnable)
			m_ImguiHandle = ImGui_ImplVulkan_AddTexture(m_DescriptorImageInfo);

		m_Desc = *info;
	}

}