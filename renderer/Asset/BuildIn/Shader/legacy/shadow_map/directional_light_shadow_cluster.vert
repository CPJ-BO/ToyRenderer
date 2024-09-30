#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_vert.glsl"
#include "../include/cluster.glsl"

layout(location = 0) out vec2 OUT_TEXCOORD;
layout(location = 1) out uint OUT_ID;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

layout(push_constant) uniform lightSetting {
    int index;
}LIGHT_SETTING;

void main()
{
    //获取cluster的顶点信息///////////////////////////////////////////////////////////////////////////////////

    uint id = fetchClusterInstanceIndex(INDIRECT_BUFFER_DIRECTIONAL_SHADOW + LIGHT_SETTING.index, gl_InstanceIndex);
    Object object = fetchClusterObject(INDIRECT_BUFFER_DIRECTIONAL_SHADOW + LIGHT_SETTING.index, gl_InstanceIndex); 
    Vertex vert = fetchClusterVertex(INDIRECT_BUFFER_DIRECTIONAL_SHADOW + LIGHT_SETTING.index, gl_VertexIndex, gl_InstanceIndex);

    vec3 POSITION = vert.pos;
    //vec3 NORMAL = vert.normal;
    //vec4 TANGENT = vert.tangent;
    //vec3 COLOR = vert.color;
    vec2 TEXCOORD = vert.texCoord;
    ivec4 BONE_IDS = vert.bone_ids;
    vec4 BONE_WEIGHTS = vert.bone_weights;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////


    //Object object = OBJECTS.slot[gl_InstanceIndex];

    DirectionalLight light = LIGHT.directionalLights[LIGHT_SETTING.index];

    vec4 pos = fetchLocalPos(object, POSITION, BONE_IDS, BONE_WEIGHTS);
    gl_Position = light.proj * light.view * object.model * pos;

    OUT_TEXCOORD    = TEXCOORD;
    OUT_ID          = id;
}