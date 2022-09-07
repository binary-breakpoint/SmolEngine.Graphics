#include "Gfx_Precompiled.h"
#include "Tools/Gfx_ShaderIncluder.h"

namespace SmolEngine
{
	Gfx_ShaderIncluder::Gfx_ShaderIncluder()
	{
		s_Instance = this;
	}

	glslang::TShader::Includer::IncludeResult* Gfx_ShaderIncluder::includeSystem(const char* headerName_, const char* includerName, size_t inclusionDepth)
	{
		const auto& headerName = std::filesystem::path(headerName_).filename().string();

		const auto& it = m_Includes.find(headerName);
		if (it != m_Includes.end())
			return it->second;

		for (const auto& includeDir : m_IncludeDirs)
		{
			if(!std::filesystem::exists(includeDir))
				continue;

			for (const auto& fDir : std::filesystem::recursive_directory_iterator(includeDir))
			{
				const auto& path = fDir.path();
				const auto& name = path.filename();

				if (headerName == name.string())
				{
					std::ifstream file(path);
					std::stringstream buffer;
					if (!file)
					{
						Gfx_Log::LogError("Could not load file " + path.string());
						return nullptr;
					}

					{
						buffer << file.rdbuf();
						file.close();

						m_Sources[headerName] = std::move(buffer.str());
					}

					const auto& src = m_Sources[headerName];
					IncResult* result = new IncResult(std::filesystem::absolute(path).string() , src.c_str(), src.length(), nullptr);

					m_Includes[headerName] = result;
					return result;
				}
			}
		}

		return nullptr;
	}

	glslang::TShader::Includer::IncludeResult* Gfx_ShaderIncluder::includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth)
	{
		return includeSystem(headerName, includerName, inclusionDepth);
	}

	void Gfx_ShaderIncluder::releaseInclude(IncResult* include)
	{
		//if (include != nullptr)
		//{
		//	std::filesystem::path path(include->headerName);
		//
		//	m_Includes.erase(path.filename().u8string());
		//	delete include;
		//}
	}

	void Gfx_ShaderIncluder::AddIncludeDir(const std::string& dir)
	{
		const bool found = std::find(s_Instance->m_IncludeDirs.begin(), 
			s_Instance->m_IncludeDirs.end(), dir) != s_Instance->m_IncludeDirs.end();

		if (!found)
			s_Instance->m_IncludeDirs.push_back(dir);
	}

	void Gfx_ShaderIncluder::Clear()
	{
		for (auto& [path, obj] : s_Instance->m_Includes)
		{
			if(obj != nullptr)
				delete obj;
		}

		s_Instance->m_Includes.clear();
		s_Instance->m_Sources.clear();
	}
}