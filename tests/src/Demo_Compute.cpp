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

	Gfx_App* context = new Gfx_App();
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
		desc.mySampler = Gfx_App::GetSampler();
		desc.myIsShaderWritable = true;
		desc.myWidth = 720;
		desc.myHeight = 480;

		storageImage.Create(&desc);
	}

	Gfx_Shader compShader;
	{
		ShaderCreateDesc shaderDesc;
		shaderDesc.myStages = { {ShaderStage::Compute, "shaders/raymarching.comp"} };
		compShader.Create(&shaderDesc);
	}

	Gfx_Descriptor compDescriptor;
	{
		DescriptorCreateDesc descriptorDesc{};
		descriptorDesc.Reflect(&compShader);

		DescriptorDesc* descriptor = descriptorDesc.GetByName("o_Image");
		descriptor->myPixelStorage = storageImage.GetPixelStorage();
		descriptor->mySampler = Gfx_App::GetSampler();

		compDescriptor.Create(&descriptorDesc);
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