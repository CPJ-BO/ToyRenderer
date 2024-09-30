#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_vert.glsl"
#include "../include/vertex.glsl"

layout(location = 0) out vec3 OUT_NORMAL;
layout(location = 1) out vec2 OUT_TEXCOORD;
layout(location = 2) out uint OUT_ID;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
    Object object   = OBJECTS.slot[gl_InstanceIndex];

    vec4 pos        = fetchLocalPos(object, POSITION, BONE_IDS, BONE_WEIGHTS);
    vec4 normal     = fetchLocalNormal(object, NORMAL, BONE_IDS, BONE_WEIGHTS);

    gl_Position     = CAMERA.proj * CAMERA.view * object.model * pos;

    OUT_NORMAL      = normalize(mat3(object.model) * normal.xyz);
    OUT_TEXCOORD    = TEXCOORD;
    OUT_ID          = gl_InstanceIndex;
}