#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../common/common.glsl"

layout(location = 0) in vec4 IN_POSITION;
layout(location = 1) in vec3 IN_COLOR;
layout(location = 2) in vec2 IN_TEXCOORD;
layout(location = 3) in vec3 IN_NORMAL;
layout(location = 4) in vec4 IN_TANGENT;
layout(location = 5) in flat uint IN_ID;

layout (location = 0) out vec4 OUT_COLOR;

void main() 
{
    Material material   = FetchMaterial(IN_ID);
    vec4 color          = vec4(IN_COLOR, 1.0f);
    vec4 diffuse        = FetchDiffuse(material, IN_TEXCOORD);
    vec3 emission       = FetchEmission(material);
    float roughness     = FetchRoughness(material, IN_TEXCOORD);
    float metallic      = FetchMetallic(material, IN_TEXCOORD);

    if(GLOBAL_SETTING.clusterInspectMode == 0) OUT_COLOR = diffuse;
    else                                       OUT_COLOR = vec4(IN_COLOR, 1.0f);
}