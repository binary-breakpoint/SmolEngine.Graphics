
#ifndef GLSL_LIGHT

#define GLSL_LIGHT 1

const int LightType_Directional = 0;
const int LightType_Point       = 1;
const int LightType_Spot        = 2;

struct Light
{
  vec3  direction;
  float range;

  vec3  color;
  float intensity;

  vec3  position;
  float innerConeCos;

  float outerConeCos;
  int   type;

  vec2 padding;
};

//-----------------------------------------------------------------------
struct BsdfSampleRec
{
  vec3  L;
  vec3  f;
  float pdf;
};

//-----------------------------------------------------------------------
struct LightSampleRec
{
  vec3  surfacePos;
  vec3  normal;
  vec3  emission;
  float pdf;
};

#endif