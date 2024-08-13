#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec4 IN_POSITION;
layout(location = 1) in vec3 IN_COLOR;
layout(location = 2) in vec2 IN_TEXCOORD;
layout(location = 3) in vec3 IN_NORMAL;
layout(location = 4) in flat uint IN_ID;

layout (location = 0) out vec4 COLOR;

#include "intersection.glsl"
#include "common.glsl"

void main() 
{
    vec3 color = texture(TEXTURES_2D[IN_ID], IN_TEXCOORD).xyz;

    // COLOR = vec4(IN_COLOR, 1.0f);
    // COLOR = vec4(IN_TEXCOORD, 0.0f, 1.0f);
    COLOR = vec4(color, 1.0f);
}