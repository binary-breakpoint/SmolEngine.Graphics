
//-------------------------------------------------------------------------------------------------
// This file is the main function for the path tracer.
// * `samplePixel()` is setting a ray from the camera origin through a pixel (jitter)
// * `PathTrace()` will loop until the ray depth is reached or the environment is hit.
// * `DirectLight()` is the contribution at the hit, if the shadow ray is not hitting anything.

#define ENVMAP 0
#define RR 1        // Using russian roulette
#define RR_DEPTH 0  // Minimum depth

#include "Include/DisneyPBR.glsl"
#include "Include/GltfMaterial.glsl"
#include "Include/Punctual.glsl"
#include "Include/ShadeState.glsl"

#define iSteps 16
#define jSteps 8

vec2 rsi(vec3 r0, vec3 rd, float sr) {
    // ray-sphere intersection that assumes
    // the sphere is centered at the origin.
    // No intersection when result.x > result.y
    float a = dot(rd, rd);
    float b = 2.0 * dot(rd, r0);
    float c = dot(r0, r0) - (sr * sr);
    float d = (b*b) - 4.0*a*c;
    if (d < 0.0) return vec2(1e5,-1e5);
    return vec2(
        (-b - sqrt(d))/(2.0*a),
        (-b + sqrt(d))/(2.0*a)
    );
}

// from: https://github.com/wwwtyro/glsl-atmosphere
vec3 atmosphere(vec3 r, vec3 r0, vec3 pSun, float iSun, float rPlanet, float rAtmos, vec3 kRlh, float kMie, float shRlh, float shMie, float g) {
    // Normalize the sun and view directions.
    pSun = normalize(pSun);
    r = normalize(r);

    // Calculate the step size of the primary ray.
    vec2 p = rsi(r0, r, rAtmos);
    if (p.x > p.y) return vec3(0,0,0);
    p.y = min(p.y, rsi(r0, r, rPlanet).x);
    float iStepSize = (p.y - p.x) / float(iSteps);

    // Initialize the primary ray time.
    float iTime = 0.0;

    // Initialize accumulators for Rayleigh and Mie scattering.
    vec3 totalRlh = vec3(0,0,0);
    vec3 totalMie = vec3(0,0,0);

    // Initialize optical depth accumulators for the primary ray.
    float iOdRlh = 0.0;
    float iOdMie = 0.0;

    // Calculate the Rayleigh and Mie phases.
    float mu = dot(r, pSun);
    float mumu = mu * mu;
    float gg = g * g;
    float pRlh = 3.0 / (16.0 * PI) * (1.0 + mumu);
    float pMie = 3.0 / (8.0 * PI) * ((1.0 - gg) * (mumu + 1.0)) / (pow(1.0 + gg - 2.0 * mu * g, 1.5) * (2.0 + gg));

    // Sample the primary ray.
    for (int i = 0; i < iSteps; i++) {

        // Calculate the primary ray sample position.
        vec3 iPos = r0 + r * (iTime + iStepSize * 0.5);

        // Calculate the height of the sample.
        float iHeight = length(iPos) - rPlanet;

        // Calculate the optical depth of the Rayleigh and Mie scattering for this step.
        float odStepRlh = exp(-iHeight / shRlh) * iStepSize;
        float odStepMie = exp(-iHeight / shMie) * iStepSize;

        // Accumulate optical depth.
        iOdRlh += odStepRlh;
        iOdMie += odStepMie;

        // Calculate the step size of the secondary ray.
        float jStepSize = rsi(iPos, pSun, rAtmos).y / float(jSteps);

        // Initialize the secondary ray time.
        float jTime = 0.0;

        // Initialize optical depth accumulators for the secondary ray.
        float jOdRlh = 0.0;
        float jOdMie = 0.0;

        // Sample the secondary ray.
        for (int j = 0; j < jSteps; j++) {

            // Calculate the secondary ray sample position.
            vec3 jPos = iPos + pSun * (jTime + jStepSize * 0.5);

            // Calculate the height of the sample.
            float jHeight = length(jPos) - rPlanet;

            // Accumulate the optical depth.
            jOdRlh += exp(-jHeight / shRlh) * jStepSize;
            jOdMie += exp(-jHeight / shMie) * jStepSize;

            // Increment the secondary ray time.
            jTime += jStepSize;
        }

        // Calculate attenuation.
        vec3 attn = exp(-(kMie * (iOdMie + jOdMie) + kRlh * (iOdRlh + jOdRlh)));

        // Accumulate scattering.
        totalRlh += odStepRlh * attn;
        totalMie += odStepMie * attn;

        // Increment the primary ray time.
        iTime += iStepSize;

    }

    // Calculate and return the final color.
    return iSun * (pRlh * kRlh * totalRlh + pMie * kMie * totalMie);
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
vec3 Eval(in State state, in vec3 V, in vec3 N, in vec3 L, inout float pdf)
{
   return DisneyEval(state, V, N, L, pdf);
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
vec3 Sample(in State state, in vec3 V, in vec3 N, inout vec3 L, inout float pdf, inout RngStateType seed)
{
    return DisneySample(state, V, N, L, pdf, seed);
}

//-----------------------------------------------------------------------
// Use for light/env contribution
struct VisibilityContribution
{
  vec3  radiance;   // Radiance at the point if light is visible
  vec3  lightDir;   // Direction to the light, to shoot shadow ray
  float lightDist;  // Distance to the light (1e32 for infinite or sky)
  bool  visible;    // true if in front of the face and should shoot shadow ray
};

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
VisibilityContribution DirectLight(in Ray r, in State state)
{
  vec3  Li = vec3(0);
  float lightPdf;
  vec3  lightContrib;
  vec3  lightDir;
  float lightDist = 1e32;
  bool  isLight   = false;

  VisibilityContribution contrib;
  contrib.radiance = vec3(0);
  contrib.visible  = false;

  // keep it simple and use either point light or environment light, each with the same
  // probability. If the environment factor is zero, we always use the point light
  // Note: see also miss shader
  float p_select_light = 1.0f;

  // in general, you would select the light depending on the importance of it
  // e.g. by incorporating their luminance

  Light lights;
  lights.direction = vec3(0);
	lights.range = 100.0f;;
	lights.color = vec3(1);
	lights.intensity = 10.0f;
	lights.position = vec3(0.0f);
	lights.innerConeCos = 15.0f;
	lights.outerConeCos = 45.0f;
	lights.type = 1;

  // Point lights
  if(RtState.numLights != 0 && rand(prd.seed) <= p_select_light)
  {
    isLight = true;

    // randomly select one of the lights
    int   light_index = int(min(rand(prd.seed) * RtState.numLights, RtState.numLights));
    Light light       = lights; //lights[light_index];

    vec3  pointToLight     = -light.direction;
    float rangeAttenuation = 1.0;
    float spotAttenuation  = 1.0;

    if(light.type != LightType_Directional)
    {
      pointToLight = light.position - state.position;
    }

    lightDist = length(pointToLight);

    // Compute range and spot light attenuation.
    if(light.type != LightType_Directional)
    {
      rangeAttenuation = getRangeAttenuation(light.range, lightDist);
    }
    if(light.type == LightType_Spot)
    {
      spotAttenuation = getSpotAttenuation(pointToLight, light.direction, light.outerConeCos, light.innerConeCos);
    }

    vec3 intensity = rangeAttenuation * spotAttenuation * light.intensity * light.color;

    lightContrib = intensity;
    lightDir     = normalize(pointToLight);
    lightPdf     = 1.0;
  }

  if(state.isSubsurface || dot(lightDir, state.ffnormal) > 0.0)
  {
    // We should shoot a ray toward the environment and check if it is not
    // occluded by an object before doing the following,
    // but the call to traceRayEXT have to store
    // live states (ex: sstate), which is really costly. So we will do
    // all the computation, but adding the contribution at the end of the
    // shader.
    // See: https://developer.nvidia.com/blog/best-practices-using-nvidia-rtx-ray-tracing/
    {
      BsdfSampleRec bsdfSampleRec;

      bsdfSampleRec.f = Eval(state, -r.direction, state.ffnormal, lightDir, bsdfSampleRec.pdf);

      float misWeight = isLight ? 1.0 : max(0.0, powerHeuristic(lightPdf, bsdfSampleRec.pdf));

      Li += misWeight * bsdfSampleRec.f * abs(dot(lightDir, state.ffnormal)) * lightContrib / lightPdf;
    }

    contrib.visible   = true;
    contrib.lightDir  = lightDir;
    contrib.lightDist = lightDist;
    contrib.radiance  = Li;
  }

  return contrib;
}

void ClosestHit(Ray r)
{
  uint rayFlags; //= gl_RayFlagsCullBackFacingTrianglesEXT;
  prd.hitT      = INFINITY;
  traceRayEXT(topLevelAS,   // acceleration structure
              rayFlags,     // rayFlags
              0xFF,         // cullMask
              0,            // sbtRecordOffset
              0,            // sbtRecordStride
              0,            // missIndex
              r.origin,     // ray origin
              0.0,          // ray min range
              r.direction,  // ray direction
              INFINITY,     // ray max range
              0             // payload (location = 0)
  );
}

//-----------------------------------------------------------------------
// Shadow ray - return true if a ray hits anything
//
bool AnyHit(Ray r, float maxDist)
{
  shadow_payload.isHit = true;      // Asume hit, will be set to false if hit nothing (miss shader)
  shadow_payload.seed  = prd.seed;  // don't care for the update - but won't affect the rahit shader
  uint rayFlags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsSkipClosestHitShaderEXT | gl_RayFlagsCullBackFacingTrianglesEXT;

  traceRayEXT(topLevelAS,   // acceleration structure
              rayFlags,     // rayFlags
              0xFF,         // cullMask
              0,            // sbtRecordOffset
              0,            // sbtRecordStride
              1,            // missIndex
              r.origin,     // ray origin
              0.0,          // ray min range
              r.direction,  // ray direction
              maxDist,      // ray max range
              1             // payload layout(location = 1)
  );

  // add to ray contribution from next event estimation
  return shadow_payload.isHit;
}

vec3 PathTrace(Ray r)
{
  vec3 radiance   = vec3(0.0);
  vec3 throughput = vec3(1.0);
  vec3 absorption = vec3(0.0);

  for(int depth = 0; depth < RtState.maxDepth; depth++)
  {
    ClosestHit(r);

    // Hitting the environment
    if(prd.hitT == INFINITY)
    {
        const vec4 rayOrigin = vec4(0, 6372e3f, 0, 0);
        const vec4 sunPosition = vec4(1.0f, 1.0f, 0, 0);
        const vec4 rayleighScatteringCoeff = vec4(5.5e-6, 13.0e-6, 22.4e-6, 0);
        const float sunIntensity = 22.0f;
        const float planetRadius = 6371e3f;
        const float atmosphereRadius = 6471e3;
        const float mieScatteringCoeff = 21e-6f;
        const float rayleighScale = 8e3f;
        const float mieScale = 1.2e3f;
        const float mieScatteringDirection = 0.758f;

        vec3 skyColor = atmosphere(
            normalize(-r.direction),         // normalized ray direction
            rayOrigin.xyz,                  // ray origin
            sunPosition.xyz,                // position of the sun
            sunIntensity,                   // intensity of the sun
            planetRadius,                   // radius of the planet in meters
            atmosphereRadius,               // radius of the atmosphere in meters
            rayleighScatteringCoeff.xyz,    // Rayleigh scattering coefficient
            mieScatteringCoeff,             // Mie scattering coefficient
            rayleighScale,                  // Rayleigh scale height
            mieScale,                       // Mie scale height
            mieScatteringDirection          // Mie preferred scattering direction
			);

       // Done sampling return
       return radiance + (skyColor * throughput);
    }

    BsdfSampleRec bsdfSampleRec;
    // Get Position, Normal, Tangents, Texture Coordinates, Color
    State state;
    GetShadeState(prd, state);
    state.ffnormal = dot(state.normal, r.direction) <= 0.0 ? state.normal : -state.normal;

    // Filling material structures
    GetMaterialsAndTextures(state, r);

    if(depth == 0)
    {
      ivec2 coods = ivec2(gl_LaunchIDEXT.xy);
    }

    // KHR_materials_unlit
    if(state.mat.unlit == 1)
    {
      return radiance + state.mat.albedo * throughput;
    }

    // Reset absorption when ray is going out of surface
    if(dot(state.normal, state.ffnormal) > 0.0)
    {
      absorption = vec3(0.0);
    }

    // Emissive material
    radiance += state.mat.emission * throughput;

    // Add absoption (transmission / volume)
    throughput *= exp(-absorption * prd.hitT);

    // Light and environment contribution
    VisibilityContribution vcontrib = DirectLight(r, state);
    vcontrib.radiance *= throughput;

      // Sampling for the next ray
    bsdfSampleRec.f = Sample(state, -r.direction, state.ffnormal, bsdfSampleRec.L, bsdfSampleRec.pdf, prd.seed);

    // Set absorption only if the ray is currently inside the object.
    if(dot(state.ffnormal, bsdfSampleRec.L) < 0.0)
    {
      absorption = -log(state.mat.attenuationColor) / vec3(state.mat.attenuationDistance);
    }

    if(bsdfSampleRec.pdf > 0.0)
    {
      throughput *= bsdfSampleRec.f * abs(dot(state.ffnormal, bsdfSampleRec.L)) / bsdfSampleRec.pdf;
    }
    else
    {
      break;
    }

#ifdef RR
    // For Russian-Roulette (minimizing live state)
    float rrPcont = (depth >= RR_DEPTH) ?
                        min(max(throughput.x, max(throughput.y, throughput.z)) * state.eta * state.eta + 0.001, 0.95) :
                        1.0;
#endif

    // Next ray
    r.direction = bsdfSampleRec.L;
    r.origin = OffsetRay(state.position, dot(bsdfSampleRec.L, state.ffnormal) > 0 ? state.ffnormal : -state.ffnormal);

    // We are adding the contribution to the radiance only if the ray is not occluded by an object.
    // This is done here to minimize live state across ray-trace calls.
    if(vcontrib.visible == true)
    {
        // Shoot shadow ray up to the light (1e32 == environement)
      Ray  shadowRay = Ray(r.origin, vcontrib.lightDir);
      bool inShadow  = AnyHit(shadowRay, vcontrib.lightDist);
      if(!inShadow)
      {
        radiance += vcontrib.radiance;
      }
    }

#ifdef RR
    if(rand(prd.seed) >= rrPcont)
      break;                // paths with low throughput that won't contribute
    throughput /= rrPcont;  // boost the energy of the non-terminated paths
#endif
  }

  // Removing fireflies
  float lum = dot(radiance, vec3(0.212671f, 0.715160f, 0.072169f));
  if(lum > 4.0)
  {
    radiance *= 4.0 / lum;
  }

  return radiance;
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

vec3 samplePixel(ivec2 imageCoords, ivec2 sizeImage)
{
	// Subpixel jitter: send the ray through a different position inside the pixel each time, to provide antialiasing.
  vec2 subpixel_jitter = RtState.frameIndex == 0 ? vec2(0.5f, 0.5f) : vec2(rand(prd.seed), rand(prd.seed));

  // Compute sampling position between [-1 .. 1]
  const vec2 pixelCenter = vec2(imageCoords) + subpixel_jitter;
  const vec2 inUV        = pixelCenter / vec2(sizeImage.xy);
  vec2       d           = inUV * 2.0 - 1.0;

	// Compute ray origin and direction
  vec4 origin    = RtState.viewInverse * vec4(0, 0, 0, 1);
  vec4 target    = RtState.projInverse * vec4(d.x, d.y, 1, 1);
  vec4 direction = RtState.viewInverse * vec4(normalize(target.xyz), 0);

  Ray ray = Ray(origin.xyz, direction.xyz);

  vec3 radiance = PathTrace(ray);

  // Removing fireflies
  //float lum = dot(radiance, vec3(0.212671f, 0.715160f, 0.072169f));
  //if(lum > rtxState.fireflyClampThreshold)
 // {
 //   radiance *= rtxState.fireflyClampThreshold / lum;
  //}

  return radiance;
}