#include "Gfx_Precompiled.h"
#include "Common/Gfx_Mesh.h"
#include "Common/Gfx_VertexBuffer.h"
#include "Common/Gfx_IndexBuffer.h"
#include "Common/Gfx_Helpers.h"

#include "Tools//Gfx_MeshImporter.h"

namespace SmolEngine
{
	Gfx_Mesh::Gfx_Mesh()
        :
        m_VertexBuffer{nullptr},
        m_IndexBuffer{nullptr},
        m_Root{this},
        m_DefaultView{nullptr},
        m_ID{0},
        m_Index{0},
        m_Name{""}
	{

	}

	void Gfx_Mesh::Free()
    {
        if (m_Scene.size() > 0)
        {
			for (auto& mesh : m_Scene)
			{
				mesh->m_VertexBuffer->Free();
				mesh->m_IndexBuffer->Free();
			}

			m_Scene.clear();
			m_Childs.clear();
        }
    }

    bool Gfx_Mesh::IsGood() const
    {
        return m_VertexBuffer->GetVertexCount() > 0;
    }

    bool Gfx_Mesh::LoadFromFile(const std::string& path, const TransformDesc& desc)
    {
        Gfx_MeshImporter::ImportedData data{};
        const bool is_succeed = Gfx_MeshImporter::Import(path, &data);
        if (is_succeed)
        {
            const uint32_t meshCount = static_cast<uint32_t>(data.myPrimitives.size());

            // Root
            {
                std::hash<std::string_view> hasher{};
                Gfx_MeshImporter::Primitive* primitve = &data.myPrimitives[0];

                m_Index = 0;
                m_AABB = primitve->myAABB;
                m_Name = primitve->myName;
                m_ID = hasher(path);

                Build(m_Root, nullptr, primitve);

                m_SceneAABB.MaxPoint(m_AABB.MaxPoint());
                m_SceneAABB.MinPoint(m_AABB.MinPoint());

                m_Scene.emplace_back(m_Root);
            }

            // Children
            uint32_t childCount = static_cast<uint32_t>(meshCount - 1);
            m_Childs.resize(childCount);
            for (uint32_t i = 0; i < childCount; ++i)
            {
                Ref<Gfx_Mesh> mesh = std::make_shared<Gfx_Mesh>();
                Gfx_MeshImporter::Primitive* primitve = &data.myPrimitives[i + 1];

                mesh->m_AABB = primitve->myAABB;
                mesh->m_Name = primitve->myName;
                mesh->m_Index = i + 1;

                Build(mesh.get(), m_Root, primitve);

                m_Childs[i] = mesh;

                m_SceneAABB.MaxPoint(mesh->m_AABB.MaxPoint());
                m_SceneAABB.MinPoint(mesh->m_AABB.MinPoint());

                m_Scene.emplace_back(mesh);
            }

            m_DefaultView = std::make_shared<Gfx_MeshView>(desc);
            m_DefaultView->m_Elements.resize(meshCount);

            //if (data.Materials.size() > 0)
            //{
            //    for (uint32_t i = 0; i < meshCount; ++i)
            //    {
            //        Primitive* primitve = &data.Primitives[i];
            //        if (primitve->MaterialIndex != -1)
            //        {
            //            auto handle = data.Materials[primitve->MaterialIndex];
            //            m_DefaultView->SetPBRHandle(handle, i);
            //        }
            //    }
            //
            //    PBRFactory::UpdateMaterials();
            //}
        }

        return is_succeed;
    }

    std::vector<Ref<Gfx_Mesh>>& Gfx_Mesh::GetScene()
    {
        return m_Scene;
    }

    std::vector<Ref<Gfx_Mesh>>& Gfx_Mesh::GetChilds()
    {
        return m_Childs;
    }

    Gfx_BoundingBox& Gfx_Mesh::GetAABB()
    {
        return m_AABB;
    }

    uint32_t Gfx_Mesh::GetChildCount() const
    {
        return static_cast<uint32_t>(m_Childs.size());
    }

    size_t Gfx_Mesh::GetID() const
    {
        return m_Root->m_ID;
    }

    uint32_t Gfx_Mesh::GetNodeIndex() const
    {
        return m_Index;
    }

    std::string Gfx_Mesh::GetName() const
    {
        return m_Name;
    }

    Ref<Gfx_MeshView> Gfx_Mesh::CreateMeshView(const TransformDesc& desc) const
    {
        Ref<Gfx_MeshView> view = std::make_shared<Gfx_MeshView>(desc);
        view->m_Elements = m_DefaultView->m_Elements;

        return view;
    }

    Ref<Gfx_VertexBuffer> Gfx_Mesh::GetVertexBuffer()
    {
        return m_VertexBuffer;
    }

    Ref<Gfx_IndexBuffer> Gfx_Mesh::GetIndexBuffer()
    {
        return m_IndexBuffer;
    }

    Ref<Gfx_Mesh> Gfx_Mesh::GetMeshByName(const std::string& name)
    {
        if (m_Root != nullptr)
            m_Root->GetMeshByName(name);

        for (auto& child : m_Childs)
        {
            if (child->GetName() == name)
                return child;
        }

        return nullptr;
    }

    Ref<Gfx_Mesh> Gfx_Mesh::GetMeshByIndex(uint32_t index)
    {
        if (index < m_Childs.size())
            return m_Childs[index];

        return nullptr;
    }

    bool Gfx_Mesh::IsRootNode() const
    {
        return m_Root == nullptr;
    }

    bool Gfx_Mesh::Build(Gfx_Mesh* mesh, Gfx_Mesh* parent, Gfx_MeshImporter::Primitive* primitive)
    {
        const bool is_static = true;

        if(parent != nullptr)
            mesh->m_Root = parent;

        mesh->m_VertexBuffer = std::make_shared<Gfx_VertexBuffer>();
        mesh->m_VertexBuffer->Create(primitive->myVertices.data(), primitive->myVertices.size() * sizeof(Gfx_MeshImporter::Vertex), is_static);

        mesh->m_IndexBuffer = std::make_shared<Gfx_IndexBuffer>();
        mesh->m_IndexBuffer->Create(primitive->myIndices.data(), primitive->myIndices.size(), is_static);

        return true;
    }

    Gfx_MeshView::Gfx_MeshView(const TransformDesc& desc)
    {
        SetTransform(desc);
    }

    bool Gfx_MeshView::TryLoadMaterials()
    {
        bool any_loaded = false;
        //for (auto& element : m_Elements)
        //{
        //    if (!element.m_PBRMatPath.empty())
        //    {
        //        //PBRCreateInfo matCI{};
        //        //if (matCI.Load(element.m_PBRMatPath))
        //        //{
        //        //    element.m_PBRHandle = PBRFactory::AddMaterial(&matCI, element.m_PBRMatPath);
        //        //    any_loaded = true;
        //        //}
        //    }
        //}

        return any_loaded;
    }

    void Gfx_MeshView::SetMask(uint32_t mask)
    {
        m_Mask = mask;
    }


    void Gfx_MeshView::SetTransform(const TransformDesc& desc)
    {
        m_PrevModelMatrix = m_ModelMatrix;
        m_ModelMatrix = Gfx_Helpers::ComposeTransform(desc.Position, desc.Rotation, desc.Scale);

        if (!m_bInitTransform)
        {
            m_PrevModelMatrix = m_ModelMatrix;
            m_bInitTransform = true;
        }
        else
        {
            m_bNeedUpdate = true;
        }
    }

    const glm::mat4& Gfx_MeshView::GetTransform() const
    {
        return m_ModelMatrix;
    }

    uint32_t Gfx_MeshView::GetMask() const
    {
        return m_Mask;
    }
}