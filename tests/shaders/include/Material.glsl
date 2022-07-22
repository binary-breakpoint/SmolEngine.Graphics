#ifndef MATERIAL_GLSL

#define MATERIAL_GLSL 1

struct Material
{
  vec3  albedo;
  float specular;

  vec3  emission;
  float anisotropy;

  float metallic;
  float roughness;
  float subsurface;
  float specularTint;

  vec3  sheenTint;
  float sheen;

  float clearcoat;
  float clearcoatRoughness;
  float transmission;
  float ior;

  vec3  attenuationColor;
  float attenuationDistance;

  vec3  f0;
  float ay;

  float alpha;
  float alphaCutoff;
  uint  forceAlpha;
  uint  doubleSided;

  uint  unlit;
  uint  thinwalled;
  float ax;
};

// From shading state, this is the structure pass to the eval functions
struct State
{
  int   depth;
  float eta;

  vec3 position;
  vec3 normal;
  vec3 ffnormal;
  vec3 tangent;
  vec3 bitangent;
  vec2 texCoord;

  bool isEmitter;
  bool specularBounce;
  bool isSubsurface;

  uint     matID;
  Material mat;
};

#endif