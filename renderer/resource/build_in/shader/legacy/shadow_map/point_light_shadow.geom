#version 450
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

layout (location = 0) in vec4 inPos[];

layout(push_constant) uniform pointLightSetting {
    int slotIndex;
}POINT_LIGHT_SETTING;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout (location = 0) out vec4 outPos;
layout (location = 1) out int outIndex;


void Emit(int index, PointLight light, vec4 pos[3])
{
    gl_Layer = index;

    gl_Position = light.proj * light.view[index] * pos[0];
    outPos = pos[0];
    outIndex = index;
    EmitVertex();

    gl_Position = light.proj * light.view[index] * pos[1];
    outPos = pos[1];
    outIndex = index;
    EmitVertex();

    gl_Position = light.proj * light.view[index] * pos[2];
    outPos = pos[2];
    outIndex = index;
    EmitVertex();

    EndPrimitive();
}

void main()
{
    PointLight light = LIGHT.pointLights[POINT_LIGHT_SETTING.slotIndex];

    //可以在顶点着色器里做预投影，估计三角面三个顶点的投射位置，但是不容易做剔除（即便三个顶点都不在一个投影方向上，生成的片元还是有可能跨越这个面，而剔除需要保守）
    Emit(0, light, inPos);
    Emit(1, light, inPos);
    Emit(2, light, inPos);
    Emit(3, light, inPos);
    Emit(4, light, inPos);
    Emit(5, light, inPos);
}