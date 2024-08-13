#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

layout(location = 0) out vec4 OUT_POSITION;
layout(location = 1) out vec3 OUT_COLOR;
layout(location = 2) out vec2 OUT_TEXCOORD;
layout(location = 3) out vec3 OUT_NORMAL;
layout(location = 4) out flat uint OUT_ID;

#include "intersection.glsl"
#include "common.glsl"

void main() 
{
    // 整个管线完全依赖bindless，由darwcall中提供的物体唯一ID(gl_InstanceIndex)和下标索引(gl_VertexIndex)来获取绘制信息
    // 目前的示例里暂未完成材质系统及完整的GPU-Driven管线的迁移，有兴趣可以自行实现，主要思路都是靠唯一索引做查找
    // 所有的旧项目shader代码见/resource/shader/legacy目录

    uint index = INDICES[gl_InstanceIndex].index[gl_VertexIndex];

    vec3 pos = vec3(POSITIONS[gl_InstanceIndex].position[3 * index], 
                    POSITIONS[gl_InstanceIndex].position[3 * index + 1], 
                    POSITIONS[gl_InstanceIndex].position[3 * index + 2]);

    vec3 color = vec3(COLORS[gl_InstanceIndex].color[3 * index], 
                      COLORS[gl_InstanceIndex].color[3 * index + 1],
                      COLORS[gl_InstanceIndex].color[3 * index + 2]);

    vec2 texCoord = vec2(TEXCOORDS[gl_InstanceIndex].texCoord[2 * index], 
                         TEXCOORDS[gl_InstanceIndex].texCoord[2 * index + 1]);      

    vec3 normal = vec3(NORMALS[gl_InstanceIndex].normal[3 * index], 
                       NORMALS[gl_InstanceIndex].normal[3 * index + 1], 
                       NORMALS[gl_InstanceIndex].normal[3 * index + 2]);          

    OUT_POSITION = vec4(pos, 1.0f);
    OUT_COLOR = normal;
    OUT_TEXCOORD = texCoord;
    OUT_NORMAL = normal;
    OUT_ID = gl_InstanceIndex;

    gl_Position = CAMERA.proj * CAMERA.view * vec4(pos, 1.0f);
}