//-------------------------------------------------------------------------------------------------
// This file is resolving the material input GltfShadeMaterial, metallic-roughness,
// specular-glossiness, textures and other thing and set the State Material values
// which are used for the shading.

#ifndef GLTFMATERIAL_GLSL
#define GLTFMATERIAL_GLSL 1

#include "Include/Common.glsl"

void GetMetallicRoughness(inout State state)
{
  float dielectricSpecular = (state.mat.ior - 1) / (state.mat.ior + 1);
  dielectricSpecular *= dielectricSpecular;

  float perceptualRoughness = state.mat.roughness;
  float metallic            = state.mat.metallic;;
  vec4  baseColor           = vec4(state.mat.albedo, 1);
  vec4  emission            = vec4(state.mat.emission, 1);
  vec3  f0                  = vec3(dielectricSpecular);

  f0 = mix(vec3(dielectricSpecular), baseColor.xyz, metallic);

  state.mat.albedo    = baseColor.rgb;
  state.mat.metallic  = metallic;
  state.mat.roughness = perceptualRoughness;
  state.mat.emission  = emission.rgb;
  state.mat.f0        = f0;
}

void GetMaterialsAndTextures(inout State state, in Ray r)
{
    mat3 TBN = mat3(state.tangent, state.bitangent, state.normal);

    GetMetallicRoughness(state);

    // Clamping roughness
    state.mat.roughness = max(state.mat.roughness, 0.001);
    state.eta     = dot(state.normal, state.ffnormal) > 0.0 ? (1.0 / state.mat.ior) : state.mat.ior;

    // Calculate anisotropic roughness along the tangent and bitangent directions
    float aspect = sqrt(1.0 - state.mat.anisotropy * 0.9);
    state.mat.ax = max(0.001, state.mat.roughness / aspect);
    state.mat.ay = max(0.001, state.mat.roughness * aspect);

    if(state.mat.anisotropy > 0)
    {
      
    }

    state.mat.clearcoatRoughness = max(state.mat.clearcoatRoughness, 0.001);
}

#endif