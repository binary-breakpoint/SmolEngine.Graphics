#version 450 core

layout (location = 0) in vec4 v_FragPos;
layout (location = 1) in vec4 v_CamPos;
layout (location = 2) in mat3 v_TBN;

layout(location = 0) out vec4 out_color;

struct PointLight 
{
    vec4 position;
    vec4 color;
	float intensity;
    float raduis;
	float bias;
};

const float PI = 3.14159265359;
const float Epsilon = 0.00001;

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}  

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlick(float cosTheta, vec3 baseReflectivity) {
	return max(baseReflectivity + (1.0 - baseReflectivity) * pow(2, (-5.55473 * cosTheta - 6.98316) * cosTheta), 0.0);
}

vec3 CalcPointLight(PointLight light, vec3 V, vec3 N, vec3 F0, vec3 albedo, float metallic, float roughness, vec3 fragPos)
{           
    float distance = length(light.position.xyz - fragPos);
	if(distance > light.raduis) return vec3(0);

	vec3 L = normalize(light.position.xyz - fragPos);
    vec3 H = normalize(V + L);
	float NdotL = max(dot(N, L), 0.0);       

    float attenuation = smoothstep(light.raduis, 0, distance);//1.0 - (distance / (light.raduis));
	attenuation = clamp(1.0 - distance/light.raduis, 0.0, 1.0); attenuation *= attenuation;
	
    vec3 radiance = vec3(1.0) * attenuation * light.intensity;
	
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);
	
    vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metallic);
	vec3 diffuseBRDF = kd * albedo;
	vec3 specularBRDF = (F * NDF * G) / max(Epsilon, 4.0 * NdotL * 0.8);      
	
	return (albedo / PI + diffuseBRDF + specularBRDF) * radiance * NdotL * light.color.rgb;
}

const float GAMMA     = 2.2;
const float INV_GAMMA = 1.0 / GAMMA;

// linear to sRGB approximation
// see http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
vec3 linearTosRGB(vec3 color)
{
  return pow(color, vec3(INV_GAMMA));
}

// ACES tone map
// see: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 toneMapACES(vec3 color)
{
  const float A = 2.51;
  const float B = 0.03;
  const float C = 2.43;
  const float D = 0.59;
  const float E = 0.14;
  return linearTosRGB(clamp((color * (A * color + B)) / (color * (C * color + D) + E), 0.0, 1.0));
}

void main()
{
    PointLight light;
    light.position = vec4(0, 2.2, 0, 0);
    light.color = vec4(0.9, 0.9, 0.9, 1.0);
    light.intensity = 10.0;
    light.raduis = 1000.0;
    light.bias = 0.01;

    vec4 normal = vec4(v_TBN[2], 1.0);
    vec3 albedo = pow(vec3(0.2), vec3(2.2));

    float metalness = 0.7f;
    float roughness = 0.3f;

    vec3 V = normalize(v_CamPos.xyz - v_FragPos.xyz);

    vec3 F0 =  vec3(0.04); 
	F0 = mix(F0, albedo, metalness);

    vec3 Lo = CalcPointLight(light, V, normal.xyz, F0, albedo, metalness, roughness, v_FragPos.xyz);

    out_color = vec4(toneMapACES(Lo), 1);
}