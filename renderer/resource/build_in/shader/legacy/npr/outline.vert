
#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_vert.glsl"
#include "../include/vertex.glsl"

out gl_PerVertex 
{
    vec4 gl_Position;   
};

layout(location = 0) out vec4 OUT_POSITION;
layout(location = 1) out vec4 OUT_PREV_POSITION;
layout(location = 2) out uint OUT_ID;

void main() {

    //TODO 这个着色器还有未知bug，启动时有可能崩溃？

    Object object = OBJECTS.slot[gl_InstanceIndex];
    MaterialInfo material = fetchMaterial(object);

    float outlineWidth = fetchMaterialFloat(material, 0);                                         //floats[0] outline width

    vec4 pos        = fetchLocalPos(object, POSITION, BONE_IDS, BONE_WEIGHTS);
    vec4 prevPos    = fetchPrevLocalPos(object, POSITION, BONE_IDS, BONE_WEIGHTS);
    vec4 normal     = fetchLocalNormal(object, COLOR, BONE_IDS, BONE_WEIGHTS);  //使用平滑法线，目前存储在颜色通道里

    vec4 world_pos = object.model * pos;
    vec4 world_normal = vec4(normalize(mat3(object.model) * normal.xyz), 1.0);

    world_pos += vec4(world_normal.xyz, 0.0f) * outlineWidth;    //沿法线偏移
    
    gl_Position = CAMERA.proj * CAMERA.view * world_pos;


    OUT_POSITION        = object.model * pos;
    OUT_PREV_POSITION   = object.prev_model * prevPos;
    OUT_ID = gl_InstanceIndex;
}









