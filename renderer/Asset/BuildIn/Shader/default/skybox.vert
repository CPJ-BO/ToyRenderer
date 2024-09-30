#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../common/common.glsl"

layout(location = 0) out vec3 OUT_CUBE_SAMPLE_VECTOR;
layout(location = 1) out flat uint OUT_ID;

void main() 
{
    uint objectID           = gl_InstanceIndex;
    uint indexOffset        = gl_VertexIndex;

    mat4 model              = FetchModel(objectID);
    uint index              = FetchIndex(objectID, indexOffset);
    vec4 pos                = FetchPos(objectID, index);
    
    OUT_CUBE_SAMPLE_VECTOR  = pos.xyz;
    OUT_ID                  = objectID;

    gl_Position = CAMERA.proj * CAMERA.view * model * pos;
}


