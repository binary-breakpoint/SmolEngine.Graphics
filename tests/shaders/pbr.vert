#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec4 a_Tangent;
layout(location = 3) in vec2 a_UV;
layout(location = 4) in ivec4 a_JointIndices;
layout(location = 5) in vec4 a_JointWeight;

layout(push_constant) uniform ConstantData
{
	mat4 view;
    mat4 proj;
};

layout (location = 0)  out vec4 v_FragPos;
layout (location = 1)  out vec4 v_CamPos;
layout (location = 2)  out mat3 v_TBN;

void main()
{
    mat4 model = mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

	vec3 normal = mat3(transpose(inverse(model))) * a_Normal;
	vec3 tangent = normalize(vec3(model * a_Tangent));
	vec3 bitangent = normalize(vec3(vec4(cross(normal, tangent), 0.0)));

	v_TBN =  mat3(tangent, bitangent, normal);
    v_FragPos = model *  vec4(a_Position, 1.0);
    v_CamPos = view[3];

    gl_Position = proj * view * v_FragPos;
}
