#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../common/common.glsl"

layout(location = 0) out vec4 OUT_POS;
layout(location = 1) out vec2 OUT_TEXCOORD;
layout(location = 2) out uint OUT_ID;

layout(push_constant) uniform point_light_setting {
    uint lightID;
}POINT_LIGHT_SETTING;

void main()
{
    uint clusterID      = MESH_CLUSTER_DRAW_INFOS.slot[gl_InstanceIndex].clusterID;
    uint objectID       = MESH_CLUSTER_DRAW_INFOS.slot[gl_InstanceIndex].objectID;
    uint indexOffset    = gl_VertexIndex + MESH_CLUSTERS.slot[clusterID].indexOffset;

    mat4 model          = FetchModel(objectID);
    uint index          = FetchIndex(objectID, indexOffset);
    vec4 pos            = FetchPos(objectID, index);
    vec2 texCoord       = FetchTexCoord(objectID, index);    

    OUT_POS             = model * pos;
    OUT_TEXCOORD        = texCoord;
    OUT_ID              = objectID;
}
