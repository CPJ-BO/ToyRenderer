#version 450
#extension GL_GOOGLE_include_directive : enable

#include "../common/common.glsl"

layout(push_constant) uniform point_light_setting {
    uint lightID;
}POINT_LIGHT_SETTING;

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;
layout(location = 0) in vec4 IN_POS[];
layout(location = 1) in vec2 IN_TEXCOORD[];
layout(location = 2) in uint IN_ID[];

layout(location = 0) out vec4 OUT_POS;
layout(location = 1) out int OUT_INDEX;
layout(location = 2) out vec2 OUT_TEXCOORD;
layout(location = 3) out uint OUT_ID;

void Emit(  in int index, 
            in PointLight light, 
            in vec4 pos[3], 
            in vec2 coord[3], 
            in uint id[3])
{
    gl_Layer = index;

    gl_Position = light.proj * light.view[index] * pos[0];
    OUT_POS         = pos[0];
    OUT_INDEX       = index;
    OUT_TEXCOORD    = coord[0];
    OUT_ID          = id[0];
    EmitVertex();

    gl_Position = light.proj * light.view[index] * pos[1];
    OUT_POS         = pos[1];
    OUT_INDEX       = index;
    OUT_TEXCOORD    = coord[1];
    OUT_ID          = id[1];
    EmitVertex();

    gl_Position = light.proj * light.view[index] * pos[2];
    OUT_POS         = pos[2];
    OUT_INDEX       = index;
    OUT_TEXCOORD    = coord[2];
    OUT_ID          = id[2];
    EmitVertex();

    EndPrimitive();
}

void main()
{
    PointLight light = LIGHTS.pointLights[POINT_LIGHT_SETTING.lightID];

    //可以在顶点着色器里做预投影，估计三角面三个顶点的投射位置，但是不容易做剔除（即便三个顶点都不在一个投影方向上，生成的片元还是有可能跨越这个面，而剔除需要保守）
    Emit(0, light, IN_POS, IN_TEXCOORD, IN_ID);
    Emit(1, light, IN_POS, IN_TEXCOORD, IN_ID);
    Emit(2, light, IN_POS, IN_TEXCOORD, IN_ID);
    Emit(3, light, IN_POS, IN_TEXCOORD, IN_ID);
    Emit(4, light, IN_POS, IN_TEXCOORD, IN_ID);
    Emit(5, light, IN_POS, IN_TEXCOORD, IN_ID);
}