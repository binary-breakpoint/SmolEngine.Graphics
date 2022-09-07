#pragma once
#include "Common/Gfx_Vertex.h"
#include "Common/Gfx_BoundingBox.h"

#include <vector>
#include <string>
#include <limits>

namespace tinygltf
{
	class Model;
}

namespace SmolEngine
{					    
	struct Primitive
	{
		int                     MaterialIndex = -1;
		std::string             MeshName = "";
		std::vector<PBRVertex>  VertexBuffer;
		std::vector<uint32_t>   IndexBuffer;
		Gfx_BoundingBox         AABB;
	};

	struct ImportedDataGlTF
	{
		//std::vector<Ref<PBRHandle>> Materials;
		std::vector<Primitive>      Primitives;
	};

	class Gfx_MeshImporter
	{
	public:

		static bool Import(const std::string& filePath, ImportedDataGlTF* out_data);
		static bool ImportInverseBindMatrices(const std::string& filePath, std::vector<glm::mat4>& matrices);

	private:
		
		static void Import(tinygltf::Model* model, ImportedDataGlTF* out_data);
	};
}