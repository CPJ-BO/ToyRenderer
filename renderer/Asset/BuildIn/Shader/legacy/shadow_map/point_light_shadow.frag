#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"


layout(push_constant) uniform pointLightSetting {
    int slotIndex;
}POINT_LIGHT_SETTING;

layout (location = 0) in vec4 inPos;
layout (location = 1) in flat int inIndex;

layout(location = 0) out vec4 color;


void main() 
{	
    PointLight light = LIGHT.pointLights[POINT_LIGHT_SETTING.slotIndex];

    float depth = length(inPos.xyz - light.pos) / light.near_far_bias.y;  
    //gl_FragDepth = depth;
    
    //color = vec4(depth, 0.0, 0.0, 1.0); //距离作为深度

    color.r = exp(light.evsm_c1c2.x * depth);
    color.g = color.r * color.r;
    color.b = exp(-light.evsm_c1c2.y * depth);
    color.a = color.b * color.b;
}