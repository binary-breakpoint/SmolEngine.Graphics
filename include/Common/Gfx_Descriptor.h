#pragma once
#include "Common/Gfx_Memory.h"
#include "Common/Gfx_Asset.h"
#include "Common/Gfx_Buffer.h"
#include "Common/Gfx_AccelStructure.h"
#include "Backend/Gfx_VulkanHelpers.h"

#include <vector>
#include <string>
#include <optional>

namespace Dia
{
	class Gfx_Buffer;
	class Gfx_Shader;
	class Gfx_Sampler;
	class Gfx_PixelStorage;

	struct DescriptorDesc
	{
		Gfx_AccelStructure* myAccelStructure;
		Gfx_PixelStorage* myPixelStorage;
		Gfx_Sampler* mySampler;
		Gfx_Buffer* myBuffer;

		uint32_t myElements = 1;
		uint32_t myBinding;
		std::string myName;
		ShaderStage myStages;
		DescriptorType myType;
	};

	struct PushConstantsDesc
	{
		uint32_t mySize = 0;
		uint32_t myOffset = 0;
		ShaderStage myStages = ShaderStage::Vertex;
	};

	struct DescriptorCreateDesc
	{
		void Add(const DescriptorDesc& desc);
		void Clear();

		void SetPushConstants(PushConstantsDesc* ps);
		void Reflect(Gfx_Shader* shader);

		DescriptorDesc* GetByIndex(uint32_t index);
		DescriptorDesc* GetByName(const char* name);

		PushConstantsDesc* myPushConstant = nullptr;
		std::vector<DescriptorDesc> myBindings;
		uint32_t myNumSets = 1;
	};

	class Gfx_Descriptor final : public Gfx_Asset
	{
		friend class Gfx_Pipeline;
		friend class Gfx_RtPipeline;
		friend class Gfx_CompPipeline;
	public:
		Gfx_Descriptor();

		void Create(DescriptorCreateDesc* desc);
		void Free() override;
		bool IsGood() const override;

		void CmdUpdateBuffer(uint32_t binding, Gfx_Buffer* buffer, DescriptorType type);
		void CmdUpdatePixelStorage(uint32_t binding, Gfx_PixelStorage* storage, DescriptorType type);
		void CmdUpdatePixelStorages(uint32_t binding, const std::vector<Gfx_PixelStorage*>& storages, DescriptorType type);

		VkDescriptorSetLayout GetLayout() const { return m_Layout; }
		VkDescriptorSet GetSet() const { return m_DescriptorSet; }
		VkDescriptorPool GetPool() const { return m_Pool; }
		std::optional<VkPushConstantRange> GetPushConstantRange() const { return m_PushConstantRange; }

	private:
		VkWriteDescriptorSet CreateImageWriteSet(const DescriptorDesc* desc, const std::vector<VkDescriptorImageInfo>& infos);
		VkWriteDescriptorSet CreateBufferWriteSet(const DescriptorDesc* desc, const VkDescriptorBufferInfo& info);

		struct BufferObject
		{
			VkDescriptorBufferInfo DesriptorInfo;
			Gfx_Buffer* Buffer;
		};

		VkDescriptorPool m_Pool;
		VkDescriptorSetLayout m_Layout;
		VkDescriptorSet m_DescriptorSet;
		std::optional<VkPushConstantRange> m_PushConstantRange;
		std::unordered_map<uint32_t, Ref<BufferObject>> m_Buffers;
		std::unordered_map<uint32_t, VkWriteDescriptorSet> m_WriteSets;
	};
}