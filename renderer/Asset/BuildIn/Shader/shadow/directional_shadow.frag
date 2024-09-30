#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../common/common.glsl"

layout(location = 0) in vec2 IN_TEXCOORD;
layout(location = 1) in flat uint IN_ID;

void main() 
{
    // Material material = FetchMaterial(IN_ID);
    // vec4 diffuse = FetchDiffuse(material, IN_TEXCOORD);
    // 透明度测试
}