//-------------------------------------------------------------------------------------------------
// This file as all constant, global values and structures not shared with CPP

#include "Include/Material.glsl"
#include "Include/Light.glsl"

#ifndef GLOBALS_GLSL
#define GLOBALS_GLSL 1

#define PI 3.14159265358979323
#define TWO_PI 6.28318530717958648
#define INFINITY 1e32
#define EPS 0.0001

//precision highp int;
precision highp float;

const float M_PI        = 3.14159265358979323846;   // pi
const float M_TWO_PI    = 6.28318530717958648;      // 2*pi
const float M_PI_2      = 1.57079632679489661923;   // pi/2
const float M_PI_4      = 0.785398163397448309616;  // pi/4
const float M_1_OVER_PI = 0.318309886183790671538;  // 1/pi
const float M_2_OVER_PI = 0.636619772367581343076;  // 2/pi


#define RngStateType uint // Random type

//-----------------------------------------------------------------------

struct Ray
{
  vec3 origin;
  vec3 direction;
};

struct PtPayload
{
  uint   seed;
  float  hitT;
  int    primitiveID;
  int    instanceID;
  int    instanceCustomIndex;
  vec2   baryCoord;
  mat4x3 objectToWorld;
  mat4x3 worldToObject;
};

struct ShadowHitPayload
{
  RngStateType seed;
  bool         isHit;
};


#endif  // GLOBALS_GLSL