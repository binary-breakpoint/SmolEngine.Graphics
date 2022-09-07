#include "Gfx_Precompiled.h"
#include "Gfx_World.h"
#include "Common/Gfx_Texture.h"

namespace SmolEngine
{
	Gfx_World* Gfx_World::s_Instance = nullptr;

	Gfx_World::Gfx_World()
	{
		s_Instance = this;

		SamplerCreateDesc samplerDesc{};
		m_Sampler.Create(&samplerDesc);

		TextureCreateDesc textureDesc{};
		textureDesc.mySampler = &m_Sampler;
		textureDesc.myHeight = 4;
		textureDesc.myWidth = 4;
		uint32_t data = 0xffffffff;
		textureDesc.myUserData = &data;

		m_Texture.Create(&textureDesc);

		textureDesc.myUsage = TextureUsage::IMAGE_2D;

		m_StorageTexture.Create(&textureDesc);
	}

	Gfx_World::~Gfx_World()
	{
		s_Instance = nullptr;
	}

	Gfx_Sampler* Gfx_World::GetSampler()
	{
		return &s_Instance->m_Sampler;
	}

	Gfx_Texture* Gfx_World::GetTexture()
	{
		return &s_Instance->m_Texture;
	}

	Gfx_Texture* Gfx_World::GetStorageTexture()
	{
		return &s_Instance->m_StorageTexture;
	}

}