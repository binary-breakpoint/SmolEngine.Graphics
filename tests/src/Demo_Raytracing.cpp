#include "Gfx_Core.h"

#include <iostream>
#include <Windows.h>

using namespace SmolEngine;

struct SceneState
{
	glm::mat4 viewInverse; 
	glm::mat4 projInverse; 

	glm::ivec2 imageSize;
	uint32_t frameIndex;
	uint32_t maxSamples;

	float focalDist;
	uint32_t maxDepth;
	uint32_t numLights;
	uint32_t pad;
};

struct RtStorage
{
	RtStorage(Gfx_Mesh* mesh)
		:
		instancesBuffer{ new Gfx_Buffer() },
		transformsBuffer{ new Gfx_Buffer() },
		pointersBuffer{ new Gfx_Buffer() },
		sceneStateBuffer{ new Gfx_Buffer() },
		BLAS{ new Gfx_AccelStructure() },
		TLAS{ new Gfx_AccelStructure() }
	{
		VkTransformMatrixKHR transformMatrix;
		glm::mat3x4 model3x4 = Gfx_Helpers::ComposeTransform3x4(glm::vec3(0), glm::vec3(0, 0, 0), glm::vec3(1));
		memcpy(&transformMatrix, &model3x4, sizeof(glm::mat3x4));

		BufferCreateDesc bufferDesc = {};
		bufferDesc.myBufferUsage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
		bufferDesc.mySize = sizeof(VkTransformMatrixKHR);
		bufferDesc.myData = &transformMatrix;

		transformsBuffer->Create(bufferDesc);

		BLAS->BuildAsBottomLevel(sizeof(Gfx_MeshImporter::Vertex), transformsBuffer.get(), mesh);

		{
			VkAccelerationStructureInstanceKHR acceleration_structure_instance{};

			acceleration_structure_instance.mask = 0xFF;
			acceleration_structure_instance.instanceShaderBindingTableRecordOffset = 0;
			acceleration_structure_instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
			acceleration_structure_instance.instanceCustomIndex = 0;
			acceleration_structure_instance.accelerationStructureReference = BLAS->GetDeviceAddress();

			glm::mat3x4 transform = Gfx_Helpers::ComposeTransform3x4(glm::vec3(0), glm::vec3(0), glm::vec3(1));
			memcpy(&acceleration_structure_instance.transform, &transform, sizeof(VkTransformMatrixKHR));

			bufferDesc = {};
			bufferDesc.myData = &acceleration_structure_instance;
			bufferDesc.mySize = sizeof(VkAccelerationStructureInstanceKHR);
			bufferDesc.myFlags = BufferCreateDesc::CreateFlags::Static;
			bufferDesc.myBufferUsage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
				| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

			instancesBuffer->Create(bufferDesc);

			bool needUpdate = false;
			TLAS->BuildAsTopLevel(instancesBuffer.get(), 1, needUpdate);
		}

		struct Pointers
		{
			uint64_t vertexAddress;
			uint64_t indexAddress;
		} pointer;

		pointer.vertexAddress = mesh->GetVertexBuffer()->GetBuffer().GetDeviceAddress();
		pointer.indexAddress = mesh->GetIndexBuffer()->GetBuffer().GetDeviceAddress();

		bufferDesc = {};
		bufferDesc.myData = &pointer;
		bufferDesc.mySize = sizeof(Pointers);
		bufferDesc.myBufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		bufferDesc.myFlags = BufferCreateDesc::CreateFlags::Static;

		pointersBuffer->Create(bufferDesc);

		bufferDesc = {};
		bufferDesc.myData = nullptr;
		bufferDesc.mySize = sizeof(SceneState);
		bufferDesc.myBufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		sceneStateBuffer->Create(bufferDesc);
	}

	Scope<Gfx_Buffer> instancesBuffer;
	Scope<Gfx_Buffer> transformsBuffer;
	Scope<Gfx_Buffer> pointersBuffer;
	Scope<Gfx_Buffer> sceneStateBuffer;

	Scope<Gfx_AccelStructure> BLAS;
	Scope<Gfx_AccelStructure> TLAS;
};

#ifdef SMOLENGINE_DIST
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
#else
int main(int argc, char** argv)
#endif 
{
#ifndef SMOLENGINE_DIST
	Gfx_Log::SetCallback([](const std::string& msg, Gfx_Log::Level level) { std::cout << msg << "\n"; });
#endif 

	Gfx_App* context = new Gfx_App();
	{
		WindowCreateDesc winDesc{};
		winDesc.myTitle = "Demo Raytracing";

		GfxContextCreateDesc contextDesc{};
		contextDesc.myWindowDesc = &winDesc;

		context->Create(&contextDesc);
	}

	if (!context->GetDevice().GetRaytracingSupport())
	{
		return EXIT_FAILURE;
	}

	Gfx_Mesh* torusMesh = new Gfx_Mesh();
	torusMesh->LoadFromFile("torus.gltf");

	RtStorage* storage = new RtStorage(torusMesh);

	Gfx_Texture storageImage;
	{
		TextureCreateDesc desc{};
		desc.myFormat = Format::B8G8R8A8_UNORM;
		desc.mySampler = Gfx_App::GetSampler();
		desc.myIsShaderWritable = true;
		desc.myWidth = 720;
		desc.myHeight = 480;

		storageImage.Create(&desc);
	}

	Gfx_Descriptor rtDescriptor;
	{
		DescriptorDesc tlasDesc{};
		tlasDesc.myBinding = 0;
		tlasDesc.myAccelStructure = storage->TLAS.get();
		tlasDesc.myStages = ShaderStage::RayGen;
		tlasDesc.myType = DescriptorType::ACCEL_STRUCTURE;

		DescriptorDesc textureDesc{};
		textureDesc.myBinding = 1;
		textureDesc.mySampler = Gfx_App::GetSampler();
		textureDesc.myPixelStorage = storageImage.GetPixelStorage();
		textureDesc.myType = DescriptorType::IMAGE_2D;
		textureDesc.myStages = ShaderStage::RayGen;

		DescriptorDesc sceneBufferDesc{};
		sceneBufferDesc.myBinding = 2;
		sceneBufferDesc.myBuffer = storage->sceneStateBuffer.get();
		sceneBufferDesc.myStages = ShaderStage::RayGen;
		sceneBufferDesc.myType = DescriptorType::UNIFORM_BUFFER;

		DescriptorDesc pointersBufferDesc;
		pointersBufferDesc.myBinding = 3;
		pointersBufferDesc.myBuffer = storage->pointersBuffer.get();
		pointersBufferDesc.myStages = ShaderStage::RayGen;
		pointersBufferDesc.myType = DescriptorType::UNIFORM_BUFFER;

		DescriptorCreateDesc descriptorDesc{};
		descriptorDesc.Add(textureDesc);
		descriptorDesc.Add(tlasDesc);
		descriptorDesc.Add(sceneBufferDesc);
		descriptorDesc.Add(pointersBufferDesc);

		rtDescriptor.Create(&descriptorDesc);
	}

	Gfx_Shader rtShader;
	{
		ShaderCreateDesc shaderDesc;
		shaderDesc.myStages = 
		{ 
			{ShaderStage::RayGen, "shaders/pathtrace.rgen"}, 
			{ShaderStage::RayMiss_0, "shaders/pathtrace.rmiss"},
			{ShaderStage::RayMiss_1, "shaders/pathtrace_shadow.rmiss"},
			{ShaderStage::RayCloseHit_0, "shaders/pathtrace.rchit"},
		};

		rtShader.Create(&shaderDesc);
	}

	Gfx_RtPipeline rtPipeline;
	{
		RtPipelineCreateDesc pipelineDesc{};
		pipelineDesc.myDescriptor = &rtDescriptor;
		pipelineDesc.myShader = &rtShader;

		rtPipeline.Create(&pipelineDesc);
	}

	EditorCameraCreateDesc cameraDesc{};
	cameraDesc.myPos = glm::vec3(0, -2, 4);
	cameraDesc.myPitch = -0.5f;
	Gfx_EditorCamera camera = Gfx_EditorCamera(&cameraDesc);

	context->SetEventCallback([&camera](Gfx_Event& event)
		{
			camera.OnEvent(event);
		});

	uint32_t frameIndex = 0;
	glm::mat4 prevView;

	while (context->IsOpen())
	{
		context->ProcessEvents();

		if (context->IsWindowMinimized())
			continue;

		float deltaTime = context->CalculateDeltaTime();
		prevView = camera.GetViewMatrix();
		camera.OnUpdate(deltaTime);

		context->BeginFrame(deltaTime);
		{
			const glm::vec2 viewSize = context->GetWindowSize();

			Gfx_CmdBuffer* cmd = context->GetCommandBuffer();

			rtPipeline.CmdBindPipeline(cmd);
			rtPipeline.CmdBindDescriptor(cmd);

			SceneState sceneState = {};
			sceneState.numLights = 1;
			sceneState.maxSamples = 30;
			sceneState.maxDepth = 10;
			sceneState.viewInverse = glm::inverse(camera.GetViewMatrix());
			sceneState.projInverse = glm::inverse(camera.GetProjection());
			sceneState.imageSize = { (uint32_t)viewSize.x, (uint32_t)viewSize.y };
			sceneState.frameIndex = frameIndex;

			storage->sceneStateBuffer->SetData(&sceneState, sizeof(SceneState));

			Gfx_RtPipeline::DispatchDesc desc;
			desc.myWidth = sceneState.imageSize.x;
			desc.myHeight = sceneState.imageSize.y;

			rtPipeline.CmdDispatch(cmd, &desc);

			Gfx_VulkanHelpers::CopyPixelStorageToSwapchain(desc.myWidth, desc.myHeight, cmd, storageImage.GetPixelStorage());
		}
		context->SwapBuffers();

		if (prevView == camera.GetViewMatrix())
		{
			if (frameIndex < UINT32_MAX)
				frameIndex++;
		}
		else
			frameIndex = 0;
	}

	return EXIT_SUCCESS;
}