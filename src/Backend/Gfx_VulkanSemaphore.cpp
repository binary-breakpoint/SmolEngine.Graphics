#include "Gfx_Precompiled.h"
#include "Backend/Gfx_VulkanSemaphore.h"


namespace SmolEngine
{
    Gfx_VulkanSemaphore::Gfx_VulkanSemaphore()
    {

    }

    void Gfx_VulkanSemaphore::Create(const Gfx_VulkanDevice* device)
    {
        VkSemaphoreCreateInfo semaphoreCI = {};
        {
            semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VK_CHECK_RESULT(vkCreateSemaphore(device->GetLogicalDevice(), &semaphoreCI, nullptr, &m_PresentComplete));
            VK_CHECK_RESULT(vkCreateSemaphore(device->GetLogicalDevice(), &semaphoreCI, nullptr, &m_RenderComplete));
        }

        VkPipelineStageFlags pipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        m_SubmitInfo = {};
        m_SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        m_SubmitInfo.pWaitDstStageMask = &pipelineStageFlags;
        m_SubmitInfo.waitSemaphoreCount = 1;
        m_SubmitInfo.pWaitSemaphores = &m_PresentComplete;
        m_SubmitInfo.signalSemaphoreCount = 1;
        m_SubmitInfo.pSignalSemaphores = &m_RenderComplete;

        VkFenceCreateInfo fenceCreateInfo = {};
        {
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            m_WaitFences.resize(Gfx_App::GetSwapchain().m_Buffers.size());

            for (auto& fence : m_WaitFences)
            {
                auto create = vkCreateFence(device->GetLogicalDevice(), &fenceCreateInfo, nullptr, &fence);
                VK_CHECK_RESULT(create);

                if (create == VK_SUCCESS)
                {
                    continue;
                }
            }
        }
    }

    void Gfx_VulkanSemaphore::CreateVkSemaphore(VkSemaphore& outSemapthore)
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        {
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        }

        VK_CHECK_RESULT(vkCreateSemaphore(Gfx_App::GetDevice().GetLogicalDevice(),
            &semaphoreCreateInfo, nullptr, &outSemapthore));
    }

    const VkSemaphore Gfx_VulkanSemaphore::GetPresentCompleteSemaphore() const
    {
        return m_PresentComplete;
    }

    const VkSemaphore Gfx_VulkanSemaphore::GetRenderCompleteSemaphore() const
    {
        return m_RenderComplete;
    }

    const std::vector<VkFence>& Gfx_VulkanSemaphore::GetVkFences() const
    {
        return m_WaitFences;
    }

    const VkSubmitInfo* Gfx_VulkanSemaphore::GetSubmitInfo() const
    {
        return &m_SubmitInfo;
    }
}