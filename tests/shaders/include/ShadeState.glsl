//-------------------------------------------------------------------------------------------------
// This file finds the geometric information used for shader.
// From the payload (hit information), retrive the position, normal, uv, ..

#ifndef SHADE_STATE_GLSL
#define SHADE_STATE_GLSL


//-----------------------------------------------------------------------
// Return the tangent and binormal from the incoming normal
//-----------------------------------------------------------------------
void CreateTangent(in vec3 N, out vec3 Nt, out vec3 Nb)
{
  Nt = normalize(((abs(N.z) > 0.99999f) ? vec3(-N.x * N.y, 1.0f - N.y * N.y, -N.y * N.z) :
                                          vec3(-N.x * N.z, -N.y * N.z, 1.0f - N.z * N.z)));
  Nb = cross(Nt, N);
}

vec2 Mix(vec2 a, vec2 b, vec2 c, vec3 barycentrics)
{
	return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec3 Mix(vec3 a, vec3 b, vec3 c, vec3 barycentrics) 
{
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

void GetShadeState(in PtPayload hstate, inout State sstate)
{
  const uint idGeo  = hstate.instanceCustomIndex;  // Geometry of this instance
  const uint idPrim = hstate.primitiveID;          // Triangle ID
  const vec3 bary   = vec3(1.0 - hstate.baryCoord.x - hstate.baryCoord.y, hstate.baryCoord.x, hstate.baryCoord.y);

  Indices  indices = Indices(objects.indexAddress);
  Vertices vertices = Vertices(objects.vertexAddress);

  // Indices of the triangle
  uvec3 ind = indices.i_[idPrim];  

  // Vertex of the triangle
  Vertex v0 = vertices.v_[ind.x];
  Vertex v1 = vertices.v_[ind.y];
  Vertex v2 = vertices.v_[ind.z];

  const uint matIndex = 0; //uint(objResource.materialUIID);
  const vec2 uv = Mix(v0.UVs, v1.UVs, v2.UVs, bary);
  const vec3 position = Mix(v0.Pos, v1.Pos, v2.Pos, bary);
  const vec3 normal = normalize(Mix(v0.Normals, v1.Normals, v2.Normals, bary));

  const vec3 world_position = vec3(hstate.objectToWorld * vec4(position, 1.0));

  vec3 world_normal = normalize(vec3(normal * hstate.worldToObject));
  vec3 geom_normal  = normalize(cross(v1.Pos - v0.Pos, v2.Pos - v0.Pos));
  vec3 wgeom_normal = normalize(vec3(geom_normal * hstate.worldToObject));

  // Tangent and Binormal
  float h0 = (floatBitsToInt(v0.UVs.y) & 1) == 1 ? 1.0f : -1.0f;  // Handiness stored in the less
  float h1 = (floatBitsToInt(v1.UVs.y) & 1) == 1 ? 1.0f : -1.0f;  // significative bit of the
  float h2 = (floatBitsToInt(v2.UVs.y) & 1) == 1 ? 1.0f : -1.0f;  // texture coord V

  vec3 tangent =  normalize(Mix(v0.Tangent.xyz, v1.Tangent.xyz, v2.Tangent.xyz, bary));

  vec3 world_tangent  = normalize(vec3(mat4(hstate.objectToWorld) * vec4(tangent.xyz, 0)));
  world_tangent       = normalize(world_tangent - dot(world_tangent, world_normal) * world_normal);
  vec3 world_binormal = cross(world_normal, world_tangent) * 1.0;

  sstate.mat            = GetMaterial(matIndex);

  sstate.normal         = world_normal;
  sstate.position       = world_position;
  sstate.texCoord       = uv;
  sstate.tangent        = world_tangent;
  sstate.bitangent      = world_binormal;
  sstate.isEmitter      = false;
  sstate.specularBounce = false;
  sstate.isSubsurface   = false;

  // Move normal to same side as geometric normal
  if(dot(sstate.normal, geom_normal) <= 0)
  {
    sstate.normal *= -1.0f;
  }

}


#endif