#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_vert.glsl"
#include "../include/vertex.glsl"

layout(location = 0) out vec2 OUT_TEXCOORD;
layout(location = 1) out uint OUT_ID;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

layout(push_constant) uniform lightSetting {
    int index;
}LIGHT_SETTING;

void main()
{

    Object object = OBJECTS.slot[gl_InstanceIndex];

    DirectionalLight light = LIGHT.directionalLights[LIGHT_SETTING.index];

    vec4 pos = fetchLocalPos(object, POSITION, BONE_IDS, BONE_WEIGHTS);
    gl_Position = light.proj * light.view * object.model * pos;

    OUT_TEXCOORD    = TEXCOORD;
    OUT_ID          = gl_InstanceIndex;
}