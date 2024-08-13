
#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_vert.glsl"
#include "../include/vertex.glsl"

layout(location = 0) out vec3 OUT_COLOR;
layout(location = 1) out vec3 OUT_NORMAL;
layout(location = 2) out vec4 OUT_POSITION;
layout(location = 3) out vec2 OUT_TEXCOORD;
layout(location = 4) out vec3 OUT_VIEW_VEC;
layout(location = 5) out vec4 OUT_TANGENT;
layout(location = 6) out vec4 OUT_PREV_POSITION;
layout(location = 7) out uint OUT_ID;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() {

    Object object = OBJECTS.slot[gl_InstanceIndex];

    vec4 pos        = fetchLocalPos(object, POSITION, BONE_IDS, BONE_WEIGHTS);
    vec4 prevPos    = fetchPrevLocalPos(object, POSITION, BONE_IDS, BONE_WEIGHTS);
    vec4 normal     = fetchLocalNormal(object, NORMAL, BONE_IDS, BONE_WEIGHTS);
    vec4 tangent    = fetchLocalTangent(object, TANGENT.xyz, BONE_IDS, BONE_WEIGHTS);

    //施密特正交化的tbn矩阵，ogl
    mat3 tbnModel   = mat3(object.model);
    vec3 T          = normalize(tbnModel * tangent.xyz);
    vec3 N          = normalize(tbnModel * normal.xyz);
    // re-orthogonalize T with respect to N
    //T             = normalize(T - dot(T, N) * N);

    OUT_COLOR           = COLOR;
    OUT_POSITION        = object.model * pos;
    OUT_TEXCOORD        = TEXCOORD;
    OUT_VIEW_VEC        = CAMERA.pos.xyz - (object.model * pos).xyz;		
    OUT_NORMAL          = N;
    OUT_TANGENT         = vec4(T, TANGENT.w);
    OUT_PREV_POSITION   = object.prev_model * prevPos;
    OUT_ID              = gl_InstanceIndex;

    gl_Position = CAMERA.proj * CAMERA.view * object.model * pos;

}








