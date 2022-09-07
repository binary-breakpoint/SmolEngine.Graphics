#include "Gfx_Precompiled.h"
#if 0
#include "Renderer/Renderer.h"
#include "Materials/PBRFactory.h"

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/json.hpp>

namespace SmolEngine
{
	PBRFactory::PBRFactory()
	{
		s_Instance = this;
	}

	PBRFactory::~PBRFactory()
	{
		s_Instance = nullptr;
	}

	Ref<PBRHandle> PBRFactory::AddMaterial(PBRCreateInfo* infoCI, const std::string& path)
	{
		Ref<PBRHandle> material = GetMaterial(path);
		if (material != nullptr)
			return material;

		std::hash<std::string_view> hasher{};
		size_t UUID = hasher(path);

		material = std::make_shared<PBRHandle>();
		material->m_Uniform.ID = static_cast<uint32_t>(UUID);
		material->m_Path = path;
		material->m_Name = path;
		if (std::filesystem::exists(path))
		{
			std::filesystem::path p(path);
			material->m_Name = p.filename().stem().string();
		}

		material->Update(infoCI);

		s_Instance->m_Mutex.lock();
		{
			s_Instance->m_IDs[path] = UUID;
			s_Instance->m_Handles[UUID] = material;
			s_Instance->m_Materials.emplace_back(material);
		}
		s_Instance->m_Mutex.unlock();

		return material;
	}

	void PBRFactory::ClearMaterials()
	{
		s_Instance->m_Materials.clear();
		s_Instance->m_IDs.clear();
		s_Instance->m_Handles.clear();

		std::vector<Ref<Texture>> textures(s_Instance->m_MaxTextures);

		auto storage = RendererStorage::GetSingleton();
		auto pipeline = storage->m_GbufferPass.GetPipeline();

		pipeline->UpdateTextures(textures, RendererStorage::GetSingleton()->m_TexturesBinding, TextureFlags::TEXTURE_2D);
	}

	void PBRFactory::UpdateMaterials()
	{
		std::vector<PBRUniform> uniforms;
		std::vector<Ref<Texture>> textures;
		GetResources(textures, uniforms);

		auto storage = RendererStorage::GetSingleton();
		auto pipeline = storage->m_GbufferPass.GetPipeline();

		pipeline->UpdateTextures(textures, RendererStorage::GetSingleton()->m_TexturesBinding, TextureFlags::TEXTURE_2D);
	}

	void PBRFactory::AddDefaultMaterial()
	{
		PBRCreateInfo ci{};
		ci.Roughness = 0.5f;
		ci.Metallic = 0.5f;

		TextureCreateInfo textureCI = {};
		const std::string& path = GraphicsContext::GetSingleton()->GetResourcesPath();
		textureCI.FilePath = path + "Textures/default_background.jpg";
		ci.SetTexture(PBRTexture::Albedo, &textureCI);

		auto material = AddMaterial(&ci, "default material");
		material->m_Uniform.ID = 0;
	}

	bool PBRFactory::RemoveMaterial(const std::string& name)
	{
		const Ref<PBRHandle>& handle = GetMaterial(name);
		if (handle)
		{
			const auto& pos = std::find(s_Instance->m_Materials.begin(), s_Instance->m_Materials.end(), handle);

			s_Instance->m_Handles.erase(static_cast<size_t>(handle->GetID()));
			s_Instance->m_Materials.erase(pos);
			s_Instance->m_IDs.erase(name);

			return true;
		}

		return false;
	}

	bool PBRFactory::IsMaterialExist(const std::string& name)
	{
		return s_Instance->m_IDs.find(name) != s_Instance->m_IDs.end();
	}

	void PBRFactory::GetResources(std::vector<Ref<Texture>>& out_textures, std::vector<PBRUniform>& out_uniforms)
	{
		out_textures.resize(s_Instance->m_MaxTextures);
		out_uniforms.resize(s_Instance->m_Materials.size());

		constexpr auto addFn = [](const Ref<Texture>& texture, uint32_t& tex_index, uint32_t& global_index, std::vector<Ref<Texture>>& out_textures)
		{
			if (texture != nullptr)
			{
				tex_index = global_index;
				out_textures[global_index] = texture;
				global_index++;
			}
		};

		{
			uint32_t index = 0;

			for (uint32_t i = 0; i < static_cast<uint32_t>(s_Instance->m_Materials.size()); ++i)
			{
				const Ref<PBRHandle>& material = s_Instance->m_Materials[i];

				addFn(material->m_Albedo, material->m_Uniform.AlbedroTexIndex, index, out_textures);
				addFn(material->m_Normal, material->m_Uniform.NormalTexIndex, index, out_textures);
				addFn(material->m_Roughness, material->m_Uniform.RoughnessTexIndex, index, out_textures);
				addFn(material->m_Metallness, material->m_Uniform.MetallicTexIndex, index, out_textures);
				addFn(material->m_Emissive, material->m_Uniform.EmissiveTexIndex, index, out_textures);

				out_uniforms[i] = material->m_Uniform;
			}
		}
	}

	uint32_t PBRFactory::GetMaterialCount()
	{
		return static_cast<uint32_t>(s_Instance->m_Materials.size());
	}

	Ref<PBRHandle> PBRFactory::GetMaterial(const std::string& name)
	{
		const auto& it = s_Instance->m_IDs.find(name);
		if (it != s_Instance->m_IDs.end())
			return GetMaterial(it->second);

		return nullptr;
	}

	Ref<PBRHandle> PBRFactory::GetMaterial(size_t UUID)
	{
		const auto& it = s_Instance->m_Handles.find(UUID);
		if (it != s_Instance->m_Handles.end())
			return it->second;

		return nullptr;
	}

	const std::vector<Ref<PBRHandle>>& PBRFactory::GetMaterials()
	{
		return s_Instance->m_Materials;
	}

	void PBRHandle::Update(PBRCreateInfo* infoCI, bool update_textures)
	{
		uint32_t id = m_Uniform.ID;

		m_Uniform = *dynamic_cast<PBRUniform*>(infoCI);
		m_Uniform.ID = id;

		constexpr auto loadFN = [](TextureCreateInfo& ci, Ref<Texture>& out_tetxure, Ref<Texture>& in_tetxure, uint32_t& out_state)
		{
			if (in_tetxure != nullptr)
			{
				out_tetxure = in_tetxure;
				out_state = 1;

				return;
			}

			if (ci.FilePath.empty())
			{
				out_tetxure = nullptr;
				out_state = 0;
				return;
			}

			ci.bSamplerEnable = false;
			out_tetxure = TexturePool::ConstructFromFile(&ci);
			out_state = 1;
		};

		if (update_textures)
		{
			loadFN(infoCI->AlbedroTex, m_Albedo, infoCI->AlbedoTexPtr, m_Uniform.UseAlbedroTex);
			loadFN(infoCI->NormalTex, m_Normal, infoCI->NormalTexPtr, m_Uniform.UseNormalTex);
			loadFN(infoCI->RoughnessTex, m_Roughness, infoCI->RoughnessTexPtr, m_Uniform.UseRoughnessTex);
			loadFN(infoCI->MetallnessTex, m_Metallness, infoCI->MetallnessTexPtr, m_Uniform.UseMetallicTex);
			loadFN(infoCI->EmissiveTex, m_Emissive, infoCI->EmissiveTexPtr, m_Uniform.UseEmissiveTex);
		}
	}

	void PBRHandle::SetTexture(const Ref<Texture>& tex, PBRTexture type)
	{
		switch (type)
		{
		case PBRTexture::Albedo:
		{
			m_Albedo = tex;
			m_Uniform.UseAlbedroTex = m_Albedo != nullptr;
			break;
		}
		case PBRTexture::Normal:
		{
			m_Normal = tex;
			m_Uniform.UseNormalTex = m_Normal != nullptr;
			break;
		}
		case PBRTexture::Metallic:
		{
			m_Metallness = tex;
			m_Uniform.UseMetallicTex = m_Metallness != nullptr;
			break;
		}
		case PBRTexture::Roughness:
		{
			m_Roughness = tex;
			m_Uniform.UseRoughnessTex = m_Roughness != nullptr;
			break;
		}
		case PBRTexture::Emissive:
		{
			m_Emissive = tex;
			m_Uniform.UseEmissiveTex = m_Emissive != nullptr;
			break;
		}
		default: break;
		}
	}

	void PBRHandle::SetRoughness(float value)
	{
		m_Uniform.Roughness = value;
	}

	void PBRHandle::SetMetallness(float value)
	{
		m_Uniform.Metallic = value;
	}

	void PBRHandle::SetEmission(const glm::vec3& value)
	{
		m_Uniform.Emission  = value;
	}

	void PBRHandle::SetAlbedo(const glm::vec3& value)
	{
		m_Uniform.Albedo = value;
	}

	const std::string& PBRHandle::GetName() const
	{
		return m_Name;
	}

	const PBRUniform& PBRHandle::GetUniform() const
	{
		return m_Uniform;
	}

	uint32_t PBRHandle::GetID() const
	{
		return m_Uniform.ID;
	}

	void PBRCreateInfo::SetTexture(PBRTexture type, const TextureCreateInfo* info)
	{
		switch (type)
		{
		case PBRTexture::Albedo:
			AlbedroTex = *info;
			break;
		case PBRTexture::Normal:
			NormalTex = *info;
			break;
		case PBRTexture::Metallic:
			MetallnessTex = *info;
			break;
		case PBRTexture::Roughness:
			RoughnessTex = *info;
			break;
		case PBRTexture::Emissive:
			EmissiveTex = *info;
			break;
		default: break;
		}
	}

	void PBRCreateInfo::SetTexture(PBRTexture type, const Ref<Texture>& texture)
	{
		switch (type)
		{
		case PBRTexture::Albedo:
			AlbedoTexPtr = texture;
			break;
		case PBRTexture::Normal:
			NormalTexPtr = texture;
			break;
		case PBRTexture::Metallic:
			MetallnessTexPtr = texture;
			break;
		case PBRTexture::Roughness:
			RoughnessTexPtr = texture;
			break;
		case PBRTexture::Emissive:
			EmissiveTexPtr = texture;
			break;
		default: break;
		}
	}

	void PBRCreateInfo::GetTextures(std::map<PBRTexture, TextureCreateInfo*>& out_hashmap)
	{
		if (AlbedroTex.FilePath.empty() == false)
		{
			out_hashmap[PBRTexture::Albedo] = &AlbedroTex;
		}

		if (NormalTex.FilePath.empty() == false)
		{
			out_hashmap[PBRTexture::Normal] = &NormalTex;
		}

		if (MetallnessTex.FilePath.empty() == false)
		{
			out_hashmap[PBRTexture::Metallic] = &MetallnessTex;
		}

		if (RoughnessTex.FilePath.empty() == false)
		{
			out_hashmap[PBRTexture::Roughness] = &RoughnessTex;
		}

		if (EmissiveTex.FilePath.empty() == false)
		{
			out_hashmap[PBRTexture::Emissive] = &EmissiveTex;
		}
	}

	bool PBRCreateInfo::Load(const std::string& filePath)
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

			input(
				
				Albedo.r, Albedo.g, Albedo.b, Occlusion,
				Emission.r, Emission.g, Emission.b, EmissionStrength,
				Metallic, Roughness, Opacity, Transmission,

				ForceAlpha, DoubleSided, UseAlbedroTex, UseNormalTex,
				UseMetallicTex, UseRoughnessTex, UseEmissiveTex,

				AlbedroTexIndex, NormalTexIndex, MetallicTexIndex,
				RoughnessTexIndex, EmissiveTexIndex, UvMultiplayer,
				MetallicRoughnessTexture, ID,

				AlbedroTex, NormalTex, MetallnessTex, RoughnessTex,
				EmissiveTex
			
			);
		}

		return true;
	}

	bool PBRCreateInfo::Save(const std::string& filePath)
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
}
#endif