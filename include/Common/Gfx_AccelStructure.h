#pragma once
#include "Common/Gfx_Memory.h"
#include "Common/Gfx_Buffer.h"

namespace SmolEngine
{
	class Gfx_Mesh;
	class Gfx_Buffer;

	class Gfx_AccelStructure
	{
	public:
		Gfx_AccelStructure();
		~Gfx_AccelStructure();

		void Free();
		bool IsGood() const;
		void BuildAsBottomLevel(uint32_t vertexStride, Gfx_Buffer* transform, Gfx_Mesh* mesh);
		void BuildAsTopLevel(Gfx_Buffer* instances, uint32_t primitiveCount, bool& out_update_descriptor);

		VkAccelerationStructureKHR GetHandle();
		Gfx_Buffer& GetBuffer();
		uint64_t GetDeviceAddress() const;
	private:
		VkAccelerationStructureKHR m_Handle;
		VkAccelerationStructureKHR m_DstHandle;
		Gfx_Buffer m_Buffer{};
		uint64_t m_DeviceAddress;
		uint32_t m_NumBLASes;
	};
}