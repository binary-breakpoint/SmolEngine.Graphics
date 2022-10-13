#pragma once

namespace SmolEngine
{
	enum class Format : int
	{
		R8_UNORM,
		R8G8_UNORM,
		R8G8B8A8_UNORM,
		B8G8R8A8_UNORM,

		D16_UNORM,
		D16_UNORM_S8_UINT,
		D32_SFLOAT,
		D32_SFLOAT_S8_UINT,
		R8_SNORM,
		R8G8_SNORM,
		R8G8B8A8_SNORM,

		R16_UNORM,
		R16G16_UNORM,
		R16G16B16A16_UNORM,

		R16_SNORM,
		R16G16_SNORM,
		R16G16B16A16_SNORM,

		R16_SFLOAT,
		R16G16_SFLOAT,
		R16G16B16_SFLOAT,
		R16G16B16A16_SFLOAT,

		R32_SFLOAT,
		R32G32_SFLOAT,
		R32G32B32_SFLOAT,
		R32G32B32A32_SFLOAT,

		R16_UINT,
		R16G16_UINT,
		R16G16B16A16_UINT,

		R32_UINT,
		R32G32_UINT,
		R32G32B32A32_UINT,

		R16_SINT,
		R16G16_SINT,
		R16G16B16A16_SINT,

		R32_SINT,
		R32G32_SINT,
		R32G32B32A32_SINT,
	};

	enum class TextureUsage : int
	{
		DEFAULT,
		ARRAY,
		CUBEMAP
	};

	enum class AddressMode : int
	{
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER,
		MIRROR_CLAMP_TO_EDGE,
	};

	enum class FilterMode : int
	{
		NEAREST,
		LINEAR,
	};

	enum class BorderColor : int
	{
		FLOAT_TRANSPARENT_BLACK,
		INT_TRANSPARENT_BLACK,
		FLOAT_OPAQUE_BLACK,
		INT_OPAQUE_BLACK,
		FLOAT_OPAQUE_WHITE,
		INT_OPAQUE_WHITE
	};

	enum class LoadOp
	{
		LOAD_OP_LOAD,
		LOAD_OP_CLEAR,
		LOAD_OP_DONT_CARE
	};

	enum class StoreOp
	{
		STORE_OP_STORE,
		STORE_OP_DONT_CARE,
		STORE_OP_NONE_KHR
	};

	enum class DepthMode
	{
		DEPTH_STENCIL,
		DEPTH_ONLY,
		STENCIL_ONLY
	};

	enum class DescriptorType
	{
		SEPARATE_SAMPLER,
		COMBINED_IMAGE_SAMPLER_2D,
		TEXTURE_2D,
		IMAGE_2D,
		UNIFORM_BUFFER,
		STORAGE_BUFFER,
		ACCEL_STRUCTURE,
	};

	enum class BlendFactor : uint16_t
	{
		NONE = 0,
		ONE,
		ZERO,
		SRC_ALPHA,
		SRC_COLOR,
		ONE_MINUS_SRC_COLOR,
		DST_COLOR,
		ONE_MINUS_DST_COLOR,
		ONE_MINUS_SRC_ALPHA,
		DST_ALPHA,
		ONE_MINUS_DST_ALPHA,
		CONSTANT_COLOR,
		ONE_MINUS_CONSTANT_COLOR,
		CONSTANT_ALPHA,
		ONE_MINUS_CONSTANT_ALPHA,
		SRC_ALPHA_SATURATE,
		SRC1_COLOR,
		ONE_MINUS_SRC1_COLOR,
		SRC1_ALPHA,
		ONE_MINUS_SRC1_ALPHA,
	};

	enum class BlendOp : uint16_t
	{
		ADD,
		SUBTRACT,
		REVERSE_SUBTRACT,
		MIN,
		MAX
	};

	enum class DrawMode : uint16_t
	{
		Triangle,
		Line,
		Fan,
		Triangle_Strip,
	};

	enum class PolygonMode : uint16_t
	{
		Fill,
		Line,
		Point
	};

	enum class CullMode : uint16_t
	{
		None,
		Back,
		Front
	};

	enum class ShaderStage : int // TODO: fix 
	{
		Vertex = 1,
		Fragment = 2,
		Compute = 4,
		Geometry = 8,

		RayGen = 16,

		RayMiss_0 = 32,
		RayMiss_1 = 64,
		RayMiss_2 = 128,
		RayMiss_3 = 256,

		RayCloseHit_0 = 512,
		RayCloseHit_1 = 1024,
		RayCloseHit_2 = 2048,
		RayCloseHit_3 = 4096,

		RayAnyHit_0 = 8192,
		RayAnyHit_1 = 16384,
		RayAnyHit_2 = 32768,
		RayAnyHit_3 = 65536,

		Callable_0 = 131072,
		Callable_1 = 262144,
		Callable_2 = 524288,
		Callable_3 = 1048576,
	};

	inline ShaderStage operator~ (ShaderStage a) { return (ShaderStage)~(int)a; }
	inline ShaderStage operator| (ShaderStage a, ShaderStage b) { return (ShaderStage)((int)a | (int)b); }
	inline ShaderStage operator& (ShaderStage a, ShaderStage b) { return (ShaderStage)((int)a & (int)b); }
	inline ShaderStage operator^ (ShaderStage a, ShaderStage b) { return (ShaderStage)((int)a ^ (int)b); }
	inline ShaderStage& operator|= (ShaderStage& a, ShaderStage b) { return (ShaderStage&)((int&)a |= (int)b); }
	inline ShaderStage& operator&= (ShaderStage& a, ShaderStage b) { return (ShaderStage&)((int&)a &= (int)b); }
	inline ShaderStage& operator^= (ShaderStage& a, ShaderStage b) { return (ShaderStage&)((int&)a ^= (int)b); }

	enum class FeaturesFlags: int
	{
		ImguiEnable        = 1,
		RendererEnable     = 2,
	};

	inline FeaturesFlags operator~ (FeaturesFlags a) { return (FeaturesFlags)~(int)a; }
	inline FeaturesFlags operator| (FeaturesFlags a, FeaturesFlags b) { return (FeaturesFlags)((int)a | (int)b); }
	inline FeaturesFlags operator& (FeaturesFlags a, FeaturesFlags b) { return (FeaturesFlags)((int)a & (int)b); }
	inline FeaturesFlags operator^ (FeaturesFlags a, FeaturesFlags b) { return (FeaturesFlags)((int)a ^ (int)b); }
	inline FeaturesFlags& operator|= (FeaturesFlags& a, FeaturesFlags b) { return (FeaturesFlags&)((int&)a |= (int)b); }
	inline FeaturesFlags& operator&= (FeaturesFlags& a, FeaturesFlags b) { return (FeaturesFlags&)((int&)a &= (int)b); }
	inline FeaturesFlags& operator^= (FeaturesFlags& a, FeaturesFlags b) { return (FeaturesFlags&)((int&)a ^= (int)b); }
}