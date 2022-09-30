#pragma once
#include "Common/Gfx_Memory.h"
#include "Common/Gfx_Flags.h"
#include "Common/Gfx_BufferLayout.h"

namespace SmolEngine
{
	class Gfx_Mesh;
	class Gfx_Shader;
	class Gfx_Texture;
	class Gfx_Context;
	class Gfx_Framebuffer;
	class Gfx_VertexBuffer;
	class Gfx_IndexBuffer;
	class Gfx_Descriptor;
	class Gfx_CmdBuffer;

	struct PipelineCreateDesc
	{
		Gfx_Shader* myShader = nullptr;
		Gfx_Framebuffer* myFramebuffer = nullptr;
		Gfx_Descriptor* myDescriptor = nullptr;
		float myMinDepth = 0.0f;
		float myMaxDepth = 1.0f;
		std::string myName = "";
		BlendFactor mySrcColorBlendFactor = BlendFactor::NONE;
		BlendFactor myDstColorBlendFactor = BlendFactor::NONE;
		BlendFactor mySrcAlphaBlendFactor = BlendFactor::NONE;
		BlendFactor myDstAlphaBlendFactor = BlendFactor::NONE;
		BlendOp myColorBlendOp = BlendOp::ADD;
		BlendOp myAlphaBlendOp = BlendOp::ADD;
		CullMode myCullMode = CullMode::Back;
		PolygonMode myPolygonMode = PolygonMode::Fill;					  
		bool myDepthTestEnabled = true;
		bool myDepthWriteEnabled = true;
		bool myDepthBiasEnabled = false;
		bool myPrimitiveRestartEnable = false;
		std::vector<DrawMode> myDrawModes = { DrawMode::Triangle };
		std::vector<Gfx_BufferLayout> myVertexInput;
	};

	class Gfx_Pipeline
	{
	public:
		Gfx_Pipeline();
		~Gfx_Pipeline();

		void CmdBeginRenderPass(Gfx_CmdBuffer* cmd);
		void CmdEndRenderPass(Gfx_CmdBuffer* cmd);
		void CmdBindPipeline(Gfx_CmdBuffer* cmd);
		void CmdBindDescriptor(Gfx_CmdBuffer* cmd, Gfx_Descriptor* another = nullptr);
		void CmdPushConstant(Gfx_CmdBuffer* cmd, ShaderStage stage, uint32_t size, const void* data);
		void CmdDrawIndexed(Gfx_CmdBuffer* cmd, Gfx_VertexBuffer* vb, Gfx_IndexBuffer* ib);
		void CmdDraw(Gfx_CmdBuffer* cmd, Gfx_VertexBuffer* vb, uint32_t vertextCount);
		void CmdDrawMeshIndexed(Gfx_CmdBuffer* cmd, Gfx_Mesh* mesh, uint32_t instances = 1);
		void CmdDrawMesh(Gfx_CmdBuffer* cmd, Gfx_Mesh* mesh, uint32_t instances = 1);    

		void Reload();
		void Free();
		void Create(PipelineCreateDesc* desc);
		void SetDrawMode(DrawMode mode);
		bool IsGood() const;
		Gfx_Shader* GetShader() const { return m_Desc.myShader; }
		Gfx_Descriptor* GetDescriptor() const { return m_Desc.myDescriptor; }
		Gfx_Framebuffer* GetFramebuffer() const { return m_Desc.myFramebuffer; }

	private:
		PipelineCreateDesc m_Desc;
		VkPipelineLayout m_Layout;
		std::unordered_map<DrawMode, VkPipeline> m_Pipelines;
		DrawMode m_DrawMode;
	};
}