#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../common/common.glsl"

layout(location = 0) out vec2 OUT_TEXCOORD;
layout(location = 1) out uint OUT_ID;

layout(push_constant) uniform light_setting {
    uint index;
}LIGHT_SETTING;

void main()
{
    DirectionalLight light = LIGHTS.directionalLights[LIGHT_SETTING.index];

    uint clusterID      = MESH_CLUSTER_DRAW_INFOS.slot[gl_InstanceIndex].clusterID;
    uint objectID       = MESH_CLUSTER_DRAW_INFOS.slot[gl_InstanceIndex].objectID;
    uint indexOffset    = gl_VertexIndex + MESH_CLUSTERS.slot[clusterID].indexOffset;

    mat4 model          = FetchModel(objectID);
    uint index          = FetchIndex(objectID, indexOffset);
    vec4 pos            = FetchPos(objectID, index);
    vec2 texCoord       = FetchTexCoord(objectID, index);    

    OUT_TEXCOORD        = texCoord;
    OUT_ID              = objectID;

    gl_Position = light.proj * light.view * model * pos;
    //gl_Position = CAMERA.proj * CAMERA.view * model * pos;
    //gl_Position = CAMERA.proj * light.view * model * pos;
    //gl_Position = light.proj * CAMERA.view * model * pos;
}