#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec4 IN_POSITION;
layout(location = 1) in vec3 IN_COLOR;
layout(location = 2) in vec2 IN_TEXCOORD;
layout(location = 3) in vec3 IN_NORMAL;
layout(location = 4) in flat uint IN_ID;

layout (location = 0) out vec4 G_BUFFER_DIFFUSE;
layout (location = 1) out vec4 G_BUFFER_NORMAL;
layout (location = 2) out vec4 G_BUFFER_ARM;
layout (location = 3) out vec4 G_BUFFER_POS;

#include "intersection.glsl"
#include "common.glsl"

void main() 
{
    vec3 color = texture(TEXTURES_2D[IN_ID], IN_TEXCOORD).xyz;

    G_BUFFER_DIFFUSE = vec4(color, 1.0f);
    G_BUFFER_NORMAL  = vec4(IN_NORMAL, 0.0f);
    G_BUFFER_ARM     = vec4(0.0f);
    G_BUFFER_POS     = IN_POSITION;
}
