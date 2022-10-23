#pragma once
#include "Common/Gfx_Framebuffer.h"
#include "Common/Gfx_Pipeline.h"
#include "Common/Gfx_Texture.h"
#include "Common/Gfx_Descriptor.h"
#include "Common/Gfx_Shader.h"
#include "Common/Gfx_Sampler.h"
#include "Common/Gfx_VertexBuffer.h"
#include "Common/Gfx_IndexBuffer.h"
#include "Common/Gfx_Mesh.h"

namespace SmolEngine
{
	struct Gfx_RenderPass
	{
		Ref<Gfx_Shader> myShader;
		Ref<Gfx_Pipeline> myPipeline;
		Ref<Gfx_Framebuffer> myRenderTarget;
		Ref<Gfx_Descriptor> myDescriptor;

		Ref<Gfx_CmdBuffer> myCmd;
	};

	struct RayDispatchDesc
	{
		std::optional<ShaderStage> myAnyHitOrCallable;
		ShaderStage myRayMiss = ShaderStage::RayMiss_0;
		ShaderStage myRayHit = ShaderStage::RayCloseHit_0;

		glm::uvec2 mySize = { 0, 0 };
	};

	class Gfx_RenderContext
	{
		Gfx_RenderContext();

	public:

		void CmdBeginRenderPass(const Ref<Gfx_RenderPass>& renderPass);
		void CmdEndRenderPass(const Ref<Gfx_RenderPass>& renderPass);

		void CmdPushConstants(const Ref<Gfx_RenderPass>& renderPass, ShaderStage stage, uint32_t size, const void* data);
		void CmdBindDescriptor(const Ref<Gfx_RenderPass>& renderPass, const Ref<Gfx_Descriptor>& another = nullptr);
		void CmdBindPipeline(const Ref<Gfx_RenderPass>& renderPass);

		void CmdRayDispatch(const Ref<Gfx_RenderPass>& renderPass, RayDispatchDesc* desc);
		void CmdDispatch(const Ref<Gfx_RenderPass>& renderPass, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ = 1);
		
		void CmdDrawIndexed(const Ref<Gfx_RenderPass>& renderPass, const Ref<Gfx_VertexBuffer>& vb, const Ref<Gfx_IndexBuffer>& ib);
		void CmdDrawMeshIndexed(const Ref<Gfx_RenderPass>& renderPass, const Ref<Gfx_Mesh>& mesh, uint32_t instances = 1);

		void CmdDraw(const Ref<Gfx_RenderPass>& renderPass, uint32_t vertexCount);
		void CmdDraw(const Ref<Gfx_RenderPass>& renderPass, const Ref<Gfx_VertexBuffer> & = nullptr);
		void CmdDrawMesh(const Ref<Gfx_RenderPass>& renderPass, const Ref<Gfx_Mesh>& mesh, uint32_t instances = 1);

		static Ref<Gfx_Buffer> CreateBuffer(BufferCreateDesc& desc, const std::string& debugName = "");
		static Ref<Gfx_Shader> CreateShader(ShaderCreateDesc& desc, const std::string& debugName = "");

		static Ref<Gfx_Framebuffer> CreateFramebuffer(FramebufferCreateDesc& desc, const std::string& debugName = "");
		static Ref<Gfx_Descriptor> CreateDescriptor(DescriptorCreateDesc& desc, const std::string& debugName = "");

		static Ref<Gfx_Texture> CreateTexture(TextureCreateDesc& desc, const std::string& debugName = "");
		static Ref<Gfx_Sampler> CreateSampler(SamplerCreateDesc& desc, const std::string& debugname = "");

		static Ref<Gfx_Mesh> CreateMesh(const std::string& filePath, const TransformDesc& transform, const std::string& debugNane = "");
		static Ref<Gfx_PixelStorage> CreatePixelStorage(PixelStorageCreateDesc& desc, const std::string& debugName = "");

		static Ref<Gfx_Pipeline> CreateGraphicsPipeline(GraphicsPipelineCreateDesc& desc, const std::string& debugName = "");
		static Ref<Gfx_Pipeline> CreateComputePipeline(ComputePipelineCreateDesc& desc, const std::string& debugName = "");
		static Ref<Gfx_Pipeline> CreateRaytarcingPipeline(RaytracingPipelineCreateDesc& desc, const std::string& debugName = "");

		static Ref<Gfx_Sampler> GetDefaultSampler();


		static Gfx_RenderContext* s_Instance;

	private:

		Ref<Gfx_Sampler> m_DefaultSampler;

		std::map<std::string, Ref<Gfx_Shader>> m_Shaders;
		std::map<std::string, Ref<Gfx_Pipeline>> m_Pipelines;
		std::map<std::string, Ref<Gfx_PixelStorage>> m_PixelStorages;
		std::map<std::string, Ref<Gfx_Buffer>> m_Buffers;
	};
}