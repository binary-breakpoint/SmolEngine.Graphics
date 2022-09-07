# SmolEngine.Graphics
![SmolEngine.Graphics](https://i.imgur.com/lILYHnN.png)

## Features
- [x] Cross-platform (Windows/Linux, WIP: Android)
- [x] Texture loading (jpg, png, tga, bmp, hdr)
- [x] Own abstraction layer on top of Vulkan API
- [x] Mesh loading (gltf)
- [x] ImGui integration
- [x] Editor camera
- [x] GLSL/HLSL support
- [x] Keyboard/mouse input
- [x] Compute pipeline
- [x] Raytacing pipeline
- [x] Logger

## How to use
```c++
#include <Gfx_Core.h>

using namespace SmolEngine.Graphics;

struct PushConstant
{
	glm::mat4 view;
	glm::mat4 proj;
};

int main(int argc, char** argv)
{
	Gfx_Context* context = new Gfx_Context(); 
	{
		WindowCreateDesc winDesc{};
		winDesc.myTitle = "Demo";
		
		GfxContextCreateDesc contextDesc{};
		contextDesc.myWindowDesc = &winDesc;
		
		context->Create(&contextDesc);	
	}

	Gfx_Shader gfxShader;
	{
		ShaderCreateDesc shaderDesc;
		shaderDesc.myStages = 
		{ 
			{ShaderStage::Vertex, "shaders/pbr.vert"}, 
			{ShaderStage::Fragment, "shaders/pbr.frag"} 
		};

		gfxShader.Create(&shaderDesc);
	}

	Gfx_Descriptor gfxDescriptor;
	{
		DescriptorDesc textureDesc{};

		textureDesc.myBinding = 0;
		textureDesc.myPixelStorage = Gfx_World::GetTexture()->GetPixelStorage();
		textureDesc.mySampler = Gfx_World::GetSampler();
		textureDesc.myStages = ShaderStage::Fragment;
		textureDesc.myType = DescriptorType::TEXTURE_2D;

		PushConstantsDesc psDesc{};
		psDesc.mySize = sizeof(PushConstant);
		psDesc.myStages = ShaderStage::Vertex;

		DescriptorCreateDesc descriptorDesc{};
		descriptorDesc.SetPushConstants(&psDesc);
		descriptorDesc.Add(textureDesc);

		gfxDescriptor.Create(&descriptorDesc);
	}

	Gfx_Pipeline gfxPipeline;
	{
		PipelineCreateDesc pipelineDesc;
		pipelineDesc.myDescriptor = &gfxDescriptor;
		pipelineDesc.myShader = &gfxShader;
		pipelineDesc.myFramebuffer = Gfx_Context::GetSingleton()->GetFramebuffer().get();
		pipelineDesc.myVertexInput =
		{
			{
				Gfx_BufferElement(Format::R32G32B32_SFLOAT, "aPos"),
				Gfx_BufferElement(Format::R32G32B32_SFLOAT, "aNormal"),
				Gfx_BufferElement(Format::R32G32B32A32_SFLOAT, "aTangent"),
				Gfx_BufferElement(Format::R32G32_SFLOAT, "aUV"),
				Gfx_BufferElement(Format::R32G32B32A32_SINT, "aJointIndices"),
				Gfx_BufferElement(Format::R32G32B32A32_SFLOAT, "aJointWeight")
			}
		};

		gfxPipeline.Create(&pipelineDesc);
	}

	Gfx_Mesh* torusMesh = new Gfx_Mesh();
	torusMesh->LoadFromFile("torus.gltf");

	EditorCameraCreateDesc cameraDesc{};
	cameraDesc.myPos = glm::vec3(0, 2, 4);
	cameraDesc.myPitch = 0.5f;
	Gfx_EditorCamera camera = Gfx_EditorCamera(&cameraDesc);

	context->SetEventCallback([&camera](Gfx_Event& event)
		{
			camera.OnEvent(event);
		});

	while (context->IsOpen())
	{
		context->ProcessEvents();

		if (context->IsWindowMinimized())
			continue;

		float deltaTime = context->CalculateDeltaTime();
		camera.OnUpdate(deltaTime);

		context->BeginFrame(deltaTime);
		{
			Gfx_CmdBuffer* cmd = context->GetCommandBuffer();

			gfxPipeline.CmdBeginRenderPass(cmd);

			gfxPipeline.CmdBindPipeline(cmd);
			gfxPipeline.CmdBindDescriptor(cmd);

			PushConstant ps{ camera.GetViewMatrix(), camera.GetProjection() };
			gfxPipeline.CmdPushConstant(cmd, ShaderStage::Vertex, sizeof(PushConstant), &ps);
			gfxPipeline.CmdDrawMeshIndexed(cmd, torusMesh);

			gfxPipeline.CmdEndRenderPass(cmd);
		}
		context->SwapBuffers();
	}

	return EXIT_SUCCESS;
}

``` 

## Building
1. Clone the repository: ```git clone https://github.com/binary-breakpoint/Dia```
2. Install [Vulkan SDK 1.3+](https://vulkan.lunarg.com/sdk/home#windows)
3. Configure the solution with Premake5: run gen-project-vs2019/22.bat 
4. Build the solution with Visual Studio 
