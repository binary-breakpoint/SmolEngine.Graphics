#pragma once
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
	class Gfx_MeshImporter
	{
	public:
		struct Vertex
		{
			glm::vec3 myPositions;
			glm::vec3 myNormals;
			glm::vec4 myTangents;
			glm::vec2 myUV;
			glm::ivec4 myJointIndices;
			glm::vec4 myJointWeight;
		};

		struct Primitive
		{
			std::string myName;
			Gfx_BoundingBox myAABB;

			std::vector<Vertex> myVertices;
			std::vector<uint32_t> myIndices;
		};

		struct ImportedData
		{
			std::vector<Primitive> myPrimitives;
		};

		static bool Import(const std::string& filePath, ImportedData* out_data);
		static bool ImportInverseBindMatrices(const std::string& filePath, std::vector<glm::mat4>& matrices);

	private:
		static void Import(tinygltf::Model* model, ImportedData* out_data);
	};
}