#pragma once
#include "Backend/Gfx_VulkanCore.h"

#include "Common/Gfx_Flags.h"
#include "Common/Gfx_Asset.h"

namespace cereal
{
	class access;
}

namespace Dia
{
	struct SamplerCreateDesc
	{
		FilterMode myMipmapMode = FilterMode::LINEAR;
		FilterMode myFilterMode = FilterMode::LINEAR;
		AddressMode myAddressMode = AddressMode::REPEAT;
		BorderColor myBorderColor = BorderColor::FLOAT_OPAQUE_WHITE;

		float myLoadBias = 1.0f;
		float myMaxLoad = 1.0f;
		float myMinLoad = 0.0f;

		bool myAnisotropyEnable = true;
		bool myCompareEnable = false;

	private:
		friend class cereal::access;

		template<typename Archive>
		void serialize(Archive& archive)
		{
			archive(myLoadBias, myMaxLoad, myMinLoad, myMipmapMode, myFilterMode, myAddressMode,
				myBorderColor, myAnisotropyEnable, myCompareEnable);
		}
	};

	class Gfx_Sampler final : public Gfx_Asset
	{
	public:
		Gfx_Sampler();
		~Gfx_Sampler();

		void Create(SamplerCreateDesc* desc);
		void Free() override;

		bool IsGood() const override { return m_Sampler != nullptr; }
		VkSampler GetSampler() const { return m_Sampler; }

	private:
		VkSampler m_Sampler;
	};
}