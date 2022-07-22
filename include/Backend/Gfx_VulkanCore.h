#pragma once
#include "Common/Gfx_Log.h"

#include <vulkan/vulkan.h>

#define VK_CHECK_RESULT(f)															\
{																					\
	VkResult res = (f);																\
	if (res != VK_SUCCESS)															\
	{																				\
		Gfx_Log::LogError("VkResult is {}, in {} at line {}", res, __FILE__, __LINE__);  \
		assert(res == VK_SUCCESS);													\
	}																				\
}