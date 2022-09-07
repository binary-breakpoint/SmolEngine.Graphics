#include "Gfx_Precompiled.h"
#include "Tools/Gfx_AnimImporter.h"

#include <regex>

namespace SmolEngine
{
	bool Gfx_AnimImporter::ImportGltf(const std::string& filePath, const std::string& exePath)
	{
		std::filesystem::path path(filePath);
		const auto& parent_path = path.parent_path().string();
		std::string command_line = exePath + "gltf2ozz.exe --file=" + filePath;
		bool ex = std::system(command_line.c_str()) == 0;
		{
			std::vector<std::string> new_files;
			for (auto& dir_entry : std::filesystem::directory_iterator(std::filesystem::current_path()))
			{
				if (dir_entry.path().extension().filename().string() == ".ozz")
					new_files.emplace_back(dir_entry.path().string());
			}

			for (auto& file : new_files)
			{
				std::filesystem::path p(file);
				std::filesystem::rename(file, parent_path + "/" + path.filename().stem().string() + "_" + p.filename().string());
			}
		}

		return ex;
	}
}