#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_tracing : require  // This is about ray tracing

#include "Include/Globals.glsl"

layout(location = 0) rayPayloadInEXT PtPayload prd;

hitAttributeEXT vec2 bary;

void main()
{  
   prd.hitT                = gl_HitTEXT;
   prd.primitiveID         = gl_PrimitiveID;
   prd.instanceID          = gl_InstanceID;
   prd.instanceCustomIndex = gl_InstanceCustomIndexEXT;
   prd.baryCoord           = bary;
   prd.objectToWorld       = gl_ObjectToWorldEXT;
   prd.worldToObject       = gl_WorldToObjectEXT;
}