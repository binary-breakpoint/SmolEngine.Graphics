#include "Include/Light.glsl"
#include "Include/Material.glsl"

#ifndef LAYOUTS_GLSL
#define LAYOUTS_GLSL 1

struct Vertex
{
	vec3  Pos;
	vec3  Normals;
	vec4  Tangent;
	vec2  UVs;
	ivec4 jointIndices;
	vec4  jointWeight;
};

struct ObjBuffer
{
	uint64_t vertexAddress;
	uint64_t indexAddress;
};

layout (binding = 3) uniform IntsBuffer { ObjBuffer objects; };

layout(buffer_reference, scalar) readonly buffer Vertices {Vertex v_[]; }; // Positions of an object
layout(buffer_reference, scalar) readonly buffer Indices {uvec3 i_[]; }; // Triangle indices

Material GetMaterial(uint id)
{
  Material material;

  	material.albedo  = vec3(0.6, 0.6, 0.6);
	material.specular = 0.5f;

	material.emission = vec3(0);
	material.anisotropy = 0.0f;

	material.metallic = 1.0f;
	material.roughness = 0.0f;;
	material.subsurface = 0.0f;
	material.specularTint = 1.0f;

	material.sheenTint = vec3(0.0f);
	material.sheen = 1.0f;

	material.clearcoat = 0.0f;
	material.clearcoatRoughness = 0.0f;
	material.transmission = 0.0f;
	material.ior = 1.45f;

	material.attenuationColor = vec3(0.0f);
	material.attenuationDistance = 0.0f;

	material.f0 = vec3(0.0f);
	material.ay = 0.0f;

	material.alpha = 1.0f;
	material.alphaCutoff = 1.0f;
	material.forceAlpha = 0;
	material.doubleSided = 0;

  material.unlit = 0;
	material.thinwalled = 0;

  return material;

}


#endif