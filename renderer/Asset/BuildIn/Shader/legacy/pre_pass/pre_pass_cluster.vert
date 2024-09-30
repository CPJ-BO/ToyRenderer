#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_vert.glsl"
#include "../include/cluster.glsl"

layout(location = 0) out vec3 OUT_NORMAL;
layout(location = 1) out vec2 OUT_TEXCOORD;
layout(location = 2) out uint OUT_ID;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{

    //获取cluster的顶点信息///////////////////////////////////////////////////////////////////////////////////

    uint id = fetchClusterInstanceIndex(INDIRECT_BUFFER_PRE_PASS, gl_InstanceIndex);
    Object object = fetchClusterObject(INDIRECT_BUFFER_PRE_PASS, gl_InstanceIndex); 
    Vertex vert = fetchClusterVertex(INDIRECT_BUFFER_PRE_PASS, gl_VertexIndex, gl_InstanceIndex);

    vec3 POSITION = vert.pos;
    vec3 NORMAL = vert.normal;
    //vec4 TANGENT = vert.tangent;
    //vec3 COLOR = vert.color;
    vec2 TEXCOORD = vert.texCoord;
    ivec4 BONE_IDS = vert.bone_ids;
    vec4 BONE_WEIGHTS = vert.bone_weights;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    //Object object   = OBJECTS.slot[gl_InstanceIndex];

    vec4 pos        = fetchLocalPos(object, POSITION, BONE_IDS, BONE_WEIGHTS);
    vec4 normal     = fetchLocalNormal(object, NORMAL, BONE_IDS, BONE_WEIGHTS);

    gl_Position     = CAMERA.proj * CAMERA.view * object.model * pos;

    OUT_NORMAL      = normalize(mat3(object.model) * normal.xyz);
    OUT_TEXCOORD    = TEXCOORD;
    OUT_ID          = id;
}