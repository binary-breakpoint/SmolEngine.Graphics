#include "Gfx_Precompiled.h"
#include "Common/Gfx_Descriptor.h"
#include "Common/Gfx_Sampler.h"
#include "Common/Gfx_PixelStorage.h"
#include "Common/Gfx_Helpers.h"

#include "Tools/Gfx_ShaderCompiler.h"

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace SmolEngine
{

	static VkDescriptorType locGetDescriptorType(DescriptorType type)
	{
		switch (type)
		{
		case DescriptorType::SEPARATE_SAMPLER:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		case DescriptorType::TEXTURE_2D:
			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		case DescriptorType::IMAGE_2D:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case DescriptorType::UNIFORM_BUFFER:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case DescriptorType::STORAGE_BUFFER:
			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		case DescriptorType::ACCEL_STRUCTURE:
			return  VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		case DescriptorType::COMBINED_IMAGE_SAMPLER_2D:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		default:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}
	}

	void DescriptorCreateDesc::Add(const DescriptorDesc& desc)
	{
		Ref< DescriptorDesc> descriptorDesc = std::make_shared< DescriptorDesc>(desc);

		myBindings.emplace_back(descriptorDesc);

		myBindingIndices[desc.myBinding] = descriptorDesc;
		if (!desc.myName.empty())
			myBindingNames[desc.myName] = descriptorDesc;
	}

	void DescriptorCreateDesc::Clear()
	{
		myBindings.clear();
	}

	void DescriptorCreateDesc::SetPushConstants(PushConstantsDesc* ps)
	{
		myPushConstant = *ps;
	}

	DescriptorDesc* DescriptorCreateDesc::GetByIndex(uint32_t index)
	{
		if (myBindings.size() < index)
			return myBindings[index].get();

		return nullptr;
	}

	DescriptorDesc* DescriptorCreateDesc::GetByName(const char* name)
	{
		DescriptorDesc* desc = nullptr;
		const auto& it = myBindingNames.find(name);
		if (it != myBindingNames.end())
			desc = it->second.get();

		GFX_ASSERT(desc)
		return desc;
	}

	void DescriptorCreateDesc::Reflect(ShaderStage stage, const std::string& shaderPath, ReflectionDesc* reflectionDesc)
	{
		const auto locAddElementFn = [this](ShaderStage stage, DescriptorType descriptorType, const spirv_cross::Compiler& compiler, const spirv_cross::Resource& res)
		{
			auto& type = compiler.get_type(res.base_type_id);

			uint32_t binding = compiler.get_decoration(res.id, spv::DecorationBinding);
			uint32_t bufferElements = static_cast<uint32_t>(type.member_types.size());

			DescriptorDesc desc;
			desc.myBinding = binding;
			desc.myElements = bufferElements == 0 ? 1 : bufferElements;
			desc.myName = res.name;
			desc.myType = descriptorType;
			desc.myStages = stage;

			const auto& it = myBindingIndices.find(binding);
			if (it != myBindingIndices.end())
			{
				GFX_ASSERT(desc.myType == it->second->myType)
				GFX_ASSERT(desc.myName == it->second->myName)
				GFX_ASSERT(desc.myElements == it->second->myElements)

				it->second->myStages |= stage;
			}
			else
			{
				Add(desc);
			}
		};

		std::vector<uint32_t> binaries;
		{
			std::string cachedPath = Gfx_Helpers::GetCachedPath(shaderPath, CachedPathType::Shader);
			if (Gfx_Helpers::IsPathValid(cachedPath))
			{
				Gfx_ShaderCompiler::LoadSPIRV(cachedPath, binaries);
			}
			else
			{
				ShaderCompileDesc compileDesc{};
				compileDesc.myStage = stage;
				compileDesc.myFilePath = shaderPath;
				if (reflectionDesc)
				{
					compileDesc.myDefines = reflectionDesc->myDefines;
					compileDesc.myIncludeDir = reflectionDesc->myIncludeDir;
				}

				Gfx_ShaderCompiler::CompileSPIRV(compileDesc, binaries);
			}
		}

		spirv_cross::Compiler compiler(binaries);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();

		for (const auto& res : resources.push_constant_buffers)
		{
			auto& type = compiler.get_type(res.base_type_id);

			PushConstantsDesc pcDesc;
			pcDesc.myOffset = 0;
			pcDesc.mySize = static_cast<uint32_t>(compiler.get_declared_struct_size(type));
			pcDesc.myStages = stage;

			SetPushConstants(&pcDesc);
		}

		for (const auto& res : resources.uniform_buffers)
		{
			locAddElementFn(stage, DescriptorType::UNIFORM_BUFFER, compiler, res);
		}

		for (const auto& res : resources.storage_buffers)
		{
			locAddElementFn(stage, DescriptorType::STORAGE_BUFFER, compiler, res);
		}

		for (const auto& res : resources.acceleration_structures)
		{
			locAddElementFn(stage, DescriptorType::ACCEL_STRUCTURE, compiler, res);
		}

		for (const auto& res : resources.sampled_images)
		{
			locAddElementFn(stage, DescriptorType::COMBINED_IMAGE_SAMPLER_2D, compiler, res);
		}

		for (const auto& res : resources.storage_images)
		{
			locAddElementFn(stage, DescriptorType::IMAGE_2D, compiler, res);
		}

		for (const auto& res : resources.separate_images)
		{
			locAddElementFn(stage, DescriptorType::TEXTURE_2D, compiler, res);
		}

		for (const auto& res : resources.separate_samplers)
		{
			locAddElementFn(stage, DescriptorType::SEPARATE_SAMPLER, compiler, res);
		}
	}

	Gfx_Descriptor::Gfx_Descriptor()
		:
		m_Pool{nullptr},
		m_Layout{nullptr},
		m_DescriptorSet{nullptr} {}

	void Gfx_Descriptor::Create(DescriptorCreateDesc* desc)
	{
		if (desc->myPushConstant.mySize > 0)
		{
			VkPushConstantRange range;
			range.size = desc->myPushConstant.mySize;
			range.stageFlags = Gfx_VulkanHelpers::GetShaderStage(desc->myPushConstant.myStages);
			range.offset = desc->myPushConstant.myOffset;

			m_PushConstantRange.emplace(range);
		}

		VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();
		std::vector<VkDescriptorSetLayoutBinding> layouts;
		std::map<DescriptorType, uint32_t> poolMap;

		for (const Ref<DescriptorDesc>& resource : desc->myBindings)
		{
			VkDescriptorSetLayoutBinding layoutBinding{};
			layoutBinding.binding = resource->myBinding;
			layoutBinding.descriptorType = locGetDescriptorType(resource->myType);
			layoutBinding.descriptorCount = resource->myElements;
			layoutBinding.stageFlags = Gfx_VulkanHelpers::GetShaderStage(resource->myStages);

			layouts.emplace_back(layoutBinding);
			poolMap[resource->myType] += resource->myElements;
		}

		std::vector<VkDescriptorPoolSize> poolSizes;

		for (auto& [type, count] : poolMap)
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.descriptorCount = count;
			poolSize.type = locGetDescriptorType(type);

			poolSizes.emplace_back(poolSize);
		}

		VkDescriptorPoolCreateInfo descriptorPoolCI{};
		{
			descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolCI.pNext = nullptr;
			descriptorPoolCI.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			descriptorPoolCI.pPoolSizes = poolSizes.data();
			descriptorPoolCI.maxSets = desc->myNumSets;

			VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &m_Pool));
		}

		VkDescriptorSetLayoutCreateInfo layoutCI{};
		{
			layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutCI.bindingCount = static_cast<uint32_t>(layouts.size());
			layoutCI.pBindings = layouts.data();

			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &layoutCI, nullptr, &m_Layout));
		}

		VkDescriptorSetAllocateInfo allocateCI{};
		{
			allocateCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateCI.descriptorPool = m_Pool;
			allocateCI.descriptorSetCount = 1;
			allocateCI.pSetLayouts = &m_Layout;

			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &allocateCI, &m_DescriptorSet));
		}

		for (const Ref<DescriptorDesc>& resource : desc->myBindings)
		{
			if (resource->myType == DescriptorType::SEPARATE_SAMPLER || resource->myType == DescriptorType::IMAGE_2D
				|| resource->myType == DescriptorType::TEXTURE_2D || resource->myType == DescriptorType::COMBINED_IMAGE_SAMPLER_2D)
			{
				GFX_ASSERT_MSG((resource->myPixelStorage || resource->mySampler), "myPixelStorage == nullptr || mySampler == nullptr")

				VkDescriptorImageInfo imageInfo{};
				imageInfo.sampler = resource->mySampler->GetSampler();
				imageInfo.imageView = resource->myPixelStorage->GetImageView();
				imageInfo.imageLayout = resource->myPixelStorage->GetImageLayout();

				std::vector<VkDescriptorImageInfo> descriptorImages;
				descriptorImages.resize(resource->myElements);
				for (uint32_t i = 0; i < resource->myElements; ++i)
				{
					descriptorImages[i] = imageInfo;
				}

				const VkWriteDescriptorSet& set = CreateImageWriteSet(resource.get(), descriptorImages);
				vkUpdateDescriptorSets(device, 1, &set, 0, nullptr);
				m_WriteSets[resource->myBinding] = set;
				continue;
			}

			if (resource->myType == DescriptorType::STORAGE_BUFFER || resource->myType == DescriptorType::UNIFORM_BUFFER)
			{
				VkDescriptorBufferInfo descriptorBufferInfo;

				const auto& it = m_Buffers.find(resource->myBinding);
				if (it == m_Buffers.end())
				{
					assert(resource->myBuffer != nullptr);

					Ref<BufferObject> obj = std::make_shared<BufferObject>();
					obj->Buffer = resource->myBuffer;
					obj->DesriptorInfo.buffer = obj->Buffer->GetRawBuffer();
					obj->DesriptorInfo.offset = 0;
					obj->DesriptorInfo.range = obj->Buffer->GetSize();

					descriptorBufferInfo = obj->DesriptorInfo;
					m_Buffers[resource->myBinding] = obj;
				}
				else
					descriptorBufferInfo = it->second->DesriptorInfo;

				const VkWriteDescriptorSet& set = CreateBufferWriteSet(resource.get(), descriptorBufferInfo);
				vkUpdateDescriptorSets(device, 1, &set, 0, nullptr);
				m_WriteSets[resource->myBinding] = set;
				continue;
			}
			 
			if (resource->myType == DescriptorType::ACCEL_STRUCTURE && resource->myAccelStructure != nullptr) // TODO: assert
			{
				VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
				descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
				descriptorAccelerationStructureInfo.accelerationStructureCount = 1;

				auto handle = resource->myAccelStructure->GetHandle();
				descriptorAccelerationStructureInfo.pAccelerationStructures = &handle;

				VkWriteDescriptorSet accelerationStructureWrite{};
				accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
				accelerationStructureWrite.dstSet = m_DescriptorSet;
				accelerationStructureWrite.dstBinding = resource->myBinding;
				accelerationStructureWrite.descriptorCount = resource->myElements;
				accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

				vkUpdateDescriptorSets(device, 1, &accelerationStructureWrite, 0, nullptr);
				m_WriteSets[resource->myBinding] = accelerationStructureWrite;
				continue;
			}
		}
	}

	void Gfx_Descriptor::Free()
	{
		m_Buffers.clear();
		m_WriteSets.clear();

		if (m_DescriptorSet != nullptr)
		{
			VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();
			vkFreeDescriptorSets(device, m_Pool, 1, &m_DescriptorSet);
			m_DescriptorSet = nullptr;
		}

		VK_DESTROY_DEVICE_HANDLE(m_Layout, vkDestroyDescriptorSetLayout);
		VK_DESTROY_DEVICE_HANDLE(m_Pool, vkDestroyDescriptorPool);
	}

	bool Gfx_Descriptor::IsGood() const
	{
		return m_Layout != nullptr;
	}

	void Gfx_Descriptor::CmdUpdateBuffer(uint32_t binding, Gfx_Buffer* buffer, DescriptorType type)
	{

	}

	void Gfx_Descriptor::CmdUpdatePixelStorage(uint32_t binding, Gfx_PixelStorage* storage, DescriptorType type)
	{

	}

	void Gfx_Descriptor::CmdUpdatePixelStorages(uint32_t binding, const std::vector<Gfx_PixelStorage*>& storages, DescriptorType type)
	{
	}

	VkWriteDescriptorSet Gfx_Descriptor::CreateImageWriteSet(const DescriptorDesc* desc, const std::vector<VkDescriptorImageInfo>& infos)
	{
		VkWriteDescriptorSet writeSet = {};
		writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet.dstSet =  m_DescriptorSet;
		writeSet.descriptorType = locGetDescriptorType(desc->myType);
		writeSet.dstArrayElement = 0;
		writeSet.dstBinding = desc->myBinding;
		writeSet.descriptorCount = static_cast<uint32_t>(infos.size());
		writeSet.pImageInfo = infos.data();

		return writeSet;
	}

	VkWriteDescriptorSet Gfx_Descriptor::CreateBufferWriteSet(const DescriptorDesc* desc, const VkDescriptorBufferInfo& info)
	{
		VkWriteDescriptorSet writeSet = {};
		writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet.dstSet = m_DescriptorSet;
		writeSet.descriptorType = locGetDescriptorType(desc->myType);
		writeSet.dstBinding = desc->myBinding;
		writeSet.dstArrayElement = 0;
		writeSet.descriptorCount = 1;
		writeSet.pBufferInfo = &info;

		return writeSet;
	}
}