#include "Gfx_Precompiled.h"
#include "Common/Gfx_CmdBuffer.h"
#include "Backend/Gfx_VulkanHelpers.h"

namespace SmolEngine
{
	Gfx_CmdBuffer::Gfx_CmdBuffer()
		:
		m_Pool{nullptr},
		m_Buffer{nullptr},
		m_ExternalPool{true},
		m_State{ State::Wait } {}

	Gfx_CmdBuffer::~Gfx_CmdBuffer()
	{
		Free();
	}

	void Gfx_CmdBuffer::Create(CmdBufferCreateDesc* desc)
	{
		VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();

		if (desc->myPool == nullptr)
		{
			auto& queueFamilyIndices = Gfx_Context::GetDevice().GetQueueFamilyIndices();

			VkCommandPoolCreateInfo poolInfo = {};

			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = queueFamilyIndices.Graphics;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			VK_CHECK_RESULT(vkCreateCommandPool(device, &poolInfo, nullptr, &m_Pool));

			m_ExternalPool = false;
		}
		else
			m_Pool = desc->myPool;

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_Pool;
		allocInfo.commandBufferCount = 1;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &allocInfo, &m_Buffer));
	}

	void Gfx_CmdBuffer::Reset()
	{
		if (!m_ExternalPool && m_Pool)
		{
			VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();
			vkResetCommandPool(device, m_Pool, 0);
		}
	}

	void Gfx_CmdBuffer::CmdBeginRecord() // TODO: add assert
	{
		if (m_State == State::Wait)
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			VK_CHECK_RESULT(vkBeginCommandBuffer(m_Buffer, &beginInfo));

			m_State = State::Record;
		}
	}

	void Gfx_CmdBuffer::CmdEndRecord() // TODO: add assert
	{
		m_State = State::NeedExecute;

		VK_CHECK_RESULT(vkEndCommandBuffer(m_Buffer));
	}

	void Gfx_CmdBuffer::Free()
	{
		VkDevice device = Gfx_Context::GetDevice().GetLogicalDevice();

		if (m_State == State::NeedExecute)
		{
			Gfx_VulkanHelpers::ExecuteCmdBuffer(this);
			m_State = State::Wait;
		}

		if (m_Buffer != nullptr)
		{
			vkFreeCommandBuffers(device, m_Pool, 1, &m_Buffer);
			m_Buffer = nullptr;
		}

		if (!m_ExternalPool && m_Pool != nullptr)
		{
			vkDestroyCommandPool(device, m_Pool, nullptr);
			m_Pool = nullptr;
		}
	}

	bool Gfx_CmdBuffer::IsGood() const
	{
		return m_State == State::Wait;
	}

	VkCommandBuffer Gfx_CmdBuffer::GetBuffer()
	{
		return m_Buffer;
	}

	VkCommandPool Gfx_CmdBuffer::GetPool()
	{
		return m_Pool;
	}
}