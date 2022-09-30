#pragma once
#include "Common/Gfx_Memory.h"
#include "Common/Gfx_IndexBuffer.h"
#include "Common/Gfx_VertexBuffer.h"

#include "Tools/Gfx_MeshImporter.h"

namespace cereal
{
	class access;
}

namespace SmolEngine
{
	class Gfx_Mesh;

	struct TransformDesc
	{
		glm::vec3 Position = glm::vec3(0);
		glm::vec3 Rotation = glm::vec3(0);
		glm::vec3 Scale = glm::vec3(1);
	};

	struct Gfx_MeshView
	{
		Gfx_MeshView(const TransformDesc& desc = TransformDesc());

		struct Element
		{
			//std::string m_PBRMatPath = "";
			//Ref<Material3D> m_Material = nullptr;
			//Ref<PBRHandle> m_PBRHandle = nullptr;
		};

		bool TryLoadMaterials();
		void SetMask(uint32_t mask);
		void SetTransform(const TransformDesc& desc);
		const glm::mat4& GetTransform() const;
		uint32_t GetMask() const;

	private:
		glm::mat4 m_ModelMatrix;
		glm::mat4 m_PrevModelMatrix;
		uint32_t m_Mask;
		std::vector<Element> m_Elements;
		bool m_bInitTransform;
		bool m_bNeedUpdate;

		friend class Gfx_Mesh;
		friend class Gfx_AccelStructure;

	};

	class Gfx_Mesh
	{
	public:	
		Gfx_Mesh();

		void Free();
		bool IsGood() const;
		bool LoadFromFile(const std::string& path, const TransformDesc& desc = TransformDesc());
								 
		std::vector<Ref<Gfx_Mesh>>& GetScene();
		std::vector<Ref<Gfx_Mesh>>& GetChilds();
		Gfx_BoundingBox& GetAABB();
		uint32_t GetChildCount() const;
		size_t GetID() const;
		uint32_t GetNodeIndex() const;
		std::string GetName() const;
		Ref<Gfx_MeshView> CreateMeshView(const TransformDesc& desc = TransformDesc()) const;
		Ref<Gfx_VertexBuffer> GetVertexBuffer();
		Ref<Gfx_IndexBuffer> GetIndexBuffer();
		Ref<Gfx_Mesh> GetMeshByName(const std::string& name);
		Ref<Gfx_Mesh> GetMeshByIndex(uint32_t index);
		bool IsRootNode() const;
							     
	private:				     
		bool Build(Gfx_Mesh* mesh, Gfx_Mesh* parent, Gfx_MeshImporter::Primitive* primitive);

	private:
		Gfx_Mesh* m_Root;
		Ref<Gfx_VertexBuffer> m_VertexBuffer;
		Ref<Gfx_IndexBuffer> m_IndexBuffer;
		Ref<Gfx_MeshView> m_DefaultView;
		Gfx_BoundingBox m_AABB;
		Gfx_BoundingBox m_SceneAABB;
		size_t m_ID;
		uint32_t m_Index;
		std::string m_Name;
		std::vector<Ref<Gfx_Mesh>> m_Childs;
		std::vector<Ref<Gfx_Mesh>> m_Scene;

		friend struct RendererStorage;
		friend struct DrawList;
		friend class GraphicsPipeline;
		friend class MeshPool;
		friend class Animator;
	};
}