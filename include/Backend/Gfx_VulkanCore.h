#pragma once

#include <vulkan/vulkan.h>

#define VK_CHECK_RESULT(f)															\
{																					\
	VkResult res = (f);																\
	if (res != VK_SUCCESS)															\
	{																				\
		assert(res == VK_SUCCESS);													\
	}																				\
}