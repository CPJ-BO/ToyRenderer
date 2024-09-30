#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../common/common.glsl"

layout(location = 0) in vec4 IN_POSITION;
layout(location = 1) in vec3 IN_COLOR;
layout(location = 2) in vec2 IN_TEXCOORD;
layout(location = 3) in vec3 IN_NORMAL;
layout(location = 4) in vec4 IN_TANGENT;
layout(location = 5) in flat uint IN_ID;

layout (location = 0) out vec4 G_BUFFER_DIFFUSE_ROUGHNESS;
layout (location = 1) out vec4 G_BUFFER_NORMAL_METALLIC;
layout (location = 2) out vec4 G_BUFFER_EMISSION;
layout (location = 3) out vec2 G_BUFFER_VELOCITY;

void main() 
{
    Material material   = FetchMaterial(IN_ID);
    vec4 color          = vec4(IN_COLOR, 1.0f);
    vec4 diffuse        = FetchDiffuse(material, IN_TEXCOORD);
    vec3 emission       = FetchEmission(material);
    vec3 normal         = FetchNormal(material, IN_TEXCOORD, IN_NORMAL, IN_TANGENT);
    
    float roughness     = FetchRoughness(material, IN_TEXCOORD);
    float metallic      = FetchMetallic(material, IN_TEXCOORD);
    
    if(GLOBAL_SETTING.clusterInspectMode == 0) G_BUFFER_DIFFUSE_ROUGHNESS = vec4(diffuse.rgb, roughness);
    else                                       G_BUFFER_DIFFUSE_ROUGHNESS = color;
    
    G_BUFFER_NORMAL_METALLIC    = vec4(normal, metallic);
    G_BUFFER_EMISSION           = vec4(emission, 0.0f);
    G_BUFFER_VELOCITY           = vec2(0.0f);   // TODO
}
