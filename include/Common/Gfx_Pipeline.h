#pragma once
#include "Common/Gfx_Memory.h"
#include "Common/Gfx_Flags.h"
#include "Common/Gfx_BufferLayout.h"

namespace SmolEngine
{
	class Gfx_Shader;
	class Gfx_Texture;
	class Gfx_Framebuffer;
	class Gfx_Descriptor;
	class Gfx_CmdBuffer;

	class Gfx_Pipeline
	{
	public:

		enum class Type
		{
			Graphics,
			Compute,
			Raytrcing
		};

		Gfx_Pipeline(Type type);
		virtual ~Gfx_Pipeline();

		virtual void Free();
		virtual void Reload() = 0;
		virtual bool IsGood() const = 0;

		Type GetType() const;
		bool IsType(Type type) const;
		VkPipelineLayout GetLayout() const;
		VkPipeline GetPipeline() const;

	protected:
		VkPipelineLayout m_Layout;
		VkPipeline m_Pipeline;
		Type m_Type;
	};

	struct GraphicsPipelineCreateDesc
	{
		Ref<Gfx_Shader> myShader = nullptr;
		Ref<Gfx_Framebuffer> myFramebuffer = nullptr;
		Ref<Gfx_Descriptor> myDescriptor = nullptr;

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
		DrawMode  myDrawMode = DrawMode::Triangle;

		bool myDepthTestEnabled = true;
		bool myDepthWriteEnabled = true;
		bool myDepthBiasEnabled = false;
		bool myPrimitiveRestartEnable = false;

		std::vector<Gfx_BufferLayout> myVertexInput;
	};

	class Gfx_GraphicsPipeline final: public Gfx_Pipeline
	{
	public:
		Gfx_GraphicsPipeline()
			:Gfx_Pipeline(Gfx_Pipeline::Type::Graphics) {}

		void Create(GraphicsPipelineCreateDesc* desc);

		virtual void Reload() override;
		virtual bool IsGood() const override;

	private:
		GraphicsPipelineCreateDesc m_Desc;
	};

	struct ComputePipelineCreateDesc
	{
		Gfx_Shader* myShader = nullptr;
		Gfx_Descriptor* myDescriptor = nullptr;
	};

	class Gfx_ComputePipeline final : public Gfx_Pipeline
	{
	public:
		Gfx_ComputePipeline()
			:Gfx_Pipeline(Gfx_Pipeline::Type::Compute) {}

		void Create(ComputePipelineCreateDesc* desc);

		virtual void Reload() override;
		virtual bool IsGood() const override;

	private:
		ComputePipelineCreateDesc m_Desc;
	};

	struct RaytracingPipelineCreateDesc
	{
		Gfx_Shader* myShader = nullptr;
		Gfx_Descriptor* myDescriptor = nullptr;
		uint32_t myMaxRayRecursionDepth = 1;
	};

	class Gfx_RaytracingPipeline final : public Gfx_Pipeline
	{
	public:
		Gfx_RaytracingPipeline()
			:Gfx_Pipeline(Gfx_Pipeline::Type::Raytrcing) {}

		void Create(RaytracingPipelineCreateDesc* desc);

		virtual void Reload() override;
		virtual bool IsGood() const override;

	private:
		RaytracingPipelineCreateDesc m_Desc;
	};
}