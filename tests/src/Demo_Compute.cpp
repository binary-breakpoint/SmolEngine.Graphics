#include "Gfx_Core.h"

#include <limits>
#include <iostream>
#include <Windows.h>

using namespace SmolEngine;

struct PushConstant
{
	glm::vec2 resolution;
	float time;
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

	Gfx_Context* context = new Gfx_Context();
	{
		WindowCreateDesc winDesc{};
		winDesc.myTitle = "Demo Compute";

		GfxContextCreateDesc contextDesc{};
		contextDesc.myWindowDesc = &winDesc;

		context->Create(&contextDesc);
	}

	Gfx_Texture storageImage;
	{
		TextureCreateDesc desc{};
		desc.myFormat = Format::B8G8R8A8_UNORM;
		desc.mySampler = Gfx_Context::GetSampler();
		desc.myUsage = TextureUsage::IMAGE_2D;
		desc.myWidth = 720;
		desc.myHeight = 480;

		storageImage.Create(&desc);
	}

	Gfx_Descriptor compDescriptor;
	{
		DescriptorDesc textureDesc{};
		textureDesc.myBinding = 0;
		textureDesc.mySampler = Gfx_Context::GetSampler();
		textureDesc.myPixelStorage = storageImage.GetPixelStorage();
		textureDesc.myType = DescriptorType::IMAGE_2D;
		textureDesc.myStages = ShaderStage::Compute;

		PushConstantsDesc psDesc{};
		psDesc.mySize = sizeof(PushConstant);
		psDesc.myStages = ShaderStage::Compute;

		DescriptorCreateDesc descriptorDesc{};
		descriptorDesc.SetPushConstants(&psDesc);
		descriptorDesc.Add(textureDesc);

		compDescriptor.Create(&descriptorDesc);
	}

	Gfx_Shader compShader;
	{
		ShaderCreateDesc shaderDesc;
		shaderDesc.myStages = { {ShaderStage::Compute, "shaders/raymarching.comp"} };
		compShader.Create(&shaderDesc);
	}

	Gfx_CompPipeline compPipeline;
	{
		CompPipelineCreateDesc compDesc;
		compDesc.myDescriptor = &compDescriptor;
		compDesc.myShader = &compShader;

		compPipeline.Create(&compDesc);
	}

	float time = 0.0f;

	while (context->IsOpen())
	{
		context->ProcessEvents();

		if (context->IsWindowMinimized())
			continue;

		float deltaTime = context->CalculateDeltaTime();

		time += 0.002f;
		if (time == 5000.0f)
			time = 0.0f;

		context->BeginFrame(deltaTime);
		{
			const glm::vec2 viewSize = context->GetWindowSize();
			const uint32_t workGroupsX = (uint32_t)glm::ceil(viewSize.x / 4.0f);
			const uint32_t workGroupsY = (uint32_t)glm::ceil(viewSize.y / 4.0f);

			Gfx_CmdBuffer* cmd = context->GetCommandBuffer();

			compPipeline.CmdBindPipeline(cmd);
			compPipeline.CmdBindDescriptor(cmd);

			PushConstant pc = { viewSize, time };
			compPipeline.CmdPushConstant(cmd, ShaderStage::Compute, sizeof(PushConstant), &pc);
			compPipeline.CmdDispatch(cmd, workGroupsX, workGroupsY);

			Gfx_VulkanHelpers::CopyPixelStorageToSwapchain((uint32_t)viewSize.x, (uint32_t)viewSize.y,
				cmd, storageImage.GetPixelStorage());
		}
		context->SwapBuffers();
	}

	return EXIT_SUCCESS;
}