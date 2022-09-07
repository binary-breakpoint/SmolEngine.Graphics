#pragma once
#include "Common/Gfx_Memory.h"
#include "Common/Gfx_Sampler.h"
#include "Common/Gfx_PixelStorage.h"
#include "Common/Gfx_Asset.h"

namespace SmolEngine
{
	struct TextureCreateDesc
	{
		void* myUserData = nullptr;
		Gfx_Sampler* mySampler = nullptr;
		uint32_t myWidth = 0;
		uint32_t myHeight = 0;
		uint32_t myMipLevels = 1;
		uint32_t myArrayLayers = 1;
		Format myFormat = Format::R8_UNORM;
		TextureUsage myUsage = TextureUsage::TEXTURE_2D;
		std::string myFilePath = "";
		bool myImGUIHandleEnable = false;

		bool Save(const std::string& filePath);
		bool Load(const std::string& filePath);

	private:
		friend class cereal::access;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(myWidth, myHeight, myMipLevels, myArrayLayers, myFormat, myFilePath, myImGUIHandleEnable, myUsage);
		}
	};

	class Gfx_Texture final: public Gfx_Asset
	{
		friend class Gfx_VulkanHelpers;
	public:
		Gfx_Texture();
		~Gfx_Texture();

		void Create(TextureCreateDesc* info);
		void Free() override;

		const VkDescriptorImageInfo& GetDescriptorImageInfo() const;
		std::pair<uint32_t, uint32_t> GetMipSize(uint32_t mip) const;
		VkDescriptorImageInfo GetMipImageView(uint32_t mip);
		Gfx_PixelStorage* GetPixelStorage();
		TextureUsage GetUsageFlags() const;
		void* GetImGuiTexture() const;
		bool IsGood() const override;
		uint32_t GetMips() const;

	private:
		void LoadEX(TextureCreateDesc* info, void* data);

		void* m_ImguiHandle;
		TextureCreateDesc m_Desc;
		Gfx_PixelStorage m_PixelStorage;
		VkDescriptorImageInfo m_DescriptorImageInfo;
		std::unordered_map<uint32_t, VkImageView> m_ImageViewMap;
	};
}
