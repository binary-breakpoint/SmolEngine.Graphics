#pragma once
#include "Common/Gfx_Sampler.h"
#include "Common/Gfx_Texture.h"

namespace Dia
{
	class Gfx_World
	{
	public:
		Gfx_World();
		~Gfx_World();

		static Gfx_Sampler* GetSampler();
		static Gfx_Texture* GetTexture();
		static Gfx_Texture* GetStorageTexture();

	private:
		static Gfx_World* s_Instance;

		Gfx_Sampler m_Sampler;
		Gfx_Texture m_Texture;
		Gfx_Texture m_StorageTexture;
	};
}