#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_vert.glsl"
#include "../include/cluster.glsl"

layout(push_constant) uniform pointLightSetting {
    int slotIndex;
}POINT_LIGHT_SETTING;

layout (location = 0) out vec4 outPos;

void main()
{

    //获取cluster的顶点信息///////////////////////////////////////////////////////////////////////////////////

    uint id = fetchClusterInstanceIndex(INDIRECT_BUFFER_POINT_SHADOW + POINT_LIGHT_SETTING.slotIndex, gl_InstanceIndex);
    Object object = fetchClusterObject(INDIRECT_BUFFER_POINT_SHADOW + POINT_LIGHT_SETTING.slotIndex, gl_InstanceIndex); 
    Vertex vert = fetchClusterVertex(INDIRECT_BUFFER_POINT_SHADOW + POINT_LIGHT_SETTING.slotIndex, gl_VertexIndex, gl_InstanceIndex);

    vec3 POSITION = vert.pos;
    //vec3 NORMAL = vert.normal;
    //vec4 TANGENT = vert.tangent;
    //vec3 COLOR = vert.color;
    //vec2 TEXCOORD = vert.texCoord;
    ivec4 BONE_IDS = vert.bone_ids;
    vec4 BONE_WEIGHTS = vert.bone_weights;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    //Object object = OBJECTS.slot[gl_InstanceIndex];

    vec4 pos = fetchLocalPos(object, POSITION, BONE_IDS, BONE_WEIGHTS);
    outPos = object.model * pos;
}

