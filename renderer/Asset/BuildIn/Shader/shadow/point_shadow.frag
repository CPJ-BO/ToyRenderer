#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../common/common.glsl"

layout(push_constant) uniform point_light_setting {
    uint lightID;
}POINT_LIGHT_SETTING;

layout(location = 0) in vec4 IN_POS;
layout(location = 1) in flat int IN_INDEX;
layout(location = 2) in vec2 IN_TEXCOORD;
layout(location = 3) in flat uint IN_ID;

layout(location = 0) out vec4 OUT_COLOR;

void main() 
{	
    PointLight light = LIGHTS.pointLights[POINT_LIGHT_SETTING.lightID];

    float depth = length(IN_POS.xyz - light.pos) / light.far;  
    //gl_FragDepth = depth;

#ifdef SHADOW_QUALITY_LOW	// 直接用深度

    OUT_COLOR = vec4(depth, 0.0, 0.0, 1.0); //距离作为深度

#else	// 使用EVSM

    OUT_COLOR.r = exp(light.c1 * depth);
    OUT_COLOR.g = OUT_COLOR.r * OUT_COLOR.r;
    OUT_COLOR.b = exp(-light.c2 * depth);
    OUT_COLOR.a = OUT_COLOR.b * OUT_COLOR.b;

#endif
}