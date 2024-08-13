#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_vert.glsl"
#include "../include/vertex.glsl"

layout(push_constant) uniform pointLightSetting {
    int slotIndex;
}POINT_LIGHT_SETTING;

layout (location = 0) out vec4 outPos;

void main()
{
   Object object = OBJECTS.slot[gl_InstanceIndex];

    vec4 pos = fetchLocalPos(object, POSITION, BONE_IDS, BONE_WEIGHTS);
    outPos = object.model * pos;
}

