#ifndef FRAG_GLSL
#define FRAG_GLSL


layout(location = 0) in vec3 COLOR;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec4 POSITION;
layout(location = 3) in vec2 TEXCOORD;
layout(location = 4) in vec3 VIEW_VEC;
layout(location = 5) in vec4 TANGENT;
layout(location = 6) in vec4 PREV_POSITION;
layout(location = 7) in flat uint ID;

#endif