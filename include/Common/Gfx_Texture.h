#pragma once
#include "Common/Gfx_Memory.h"
#include "Common/Gfx_Sampler.h"
#include "Common/Gfx_PixelStorage.h"
#include "Common/Gfx_Flags.h"

#include <string>
#include <unordered_map>

namespace SmolEngine
{
	struct TextureCreateDesc
	{
		void* myUserData = nullptr;
		Ref<Gfx_Sampler> mySampler = nullptr;
		glm::uvec2 mySize = { 0, 0 };
		uint32_t myMipLevels = 1;
		uint32_t myArrayLayers = 1;
		Format myFormat = Format::R8_UNORM;
		TextureUsage myUsage = TextureUsage::DEFAULT;
		std::string myFilePath;
		bool myImGUIHandleEnable = false;
		bool myIsShaderWritable = false;
	};

	class Gfx_Texture
	{
		friend class Gfx_VulkanHelpers;
	public:
		Gfx_Texture();
		~Gfx_Texture();

		void Create(TextureCreateDesc* info);
		void Free();

		const VkDescriptorImageInfo& GetDescriptorImageInfo() const;
		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const;
		VkDescriptorImageInfo GetMipImageView(uint32_t mip);
		Ref<Gfx_PixelStorage> GetPixelStorage();
		TextureUsage GetUsageFlags() const;
		void* GetImGuiTexture() const;
		uint32_t GetMips() const;
		bool IsGood() const;

	private:
		void LoadEX(TextureCreateDesc* info, void* data);

		void* m_ImguiHandle;
		Ref<Gfx_PixelStorage> m_PixelStorage;

		TextureCreateDesc m_Desc;
		VkDescriptorImageInfo m_DescriptorImageInfo;
		std::unordered_map<uint32_t, VkImageView> m_ImageViewMap;
	};
}
