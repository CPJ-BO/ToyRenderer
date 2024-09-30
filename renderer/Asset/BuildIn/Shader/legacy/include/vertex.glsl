#ifndef VERTEX_GLSL
#define VERTEX_GLSL


layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec4 TANGENT;
layout(location = 3) in vec3 COLOR;
layout(location = 4) in vec2 TEXCOORD;
layout(location = 5) in ivec4 BONE_IDS;
layout(location = 6) in vec4 BONE_WEIGHTS;

// layout(location = 0) out vec3 OUT_COLOR;
// layout(location = 1) out vec3 OUT_NORMAL;
// layout(location = 2) out vec4 OUT_POSITION;
// layout(location = 3) out vec2 OUT_TEXCOORD;
// layout(location = 4) out vec3 OUT_VIEW_VEC;
// layout(location = 5) out vec4 OUT_TANGENT;
// layout(location = 6) out vec4 OUT_PREV_POSITION;
// layout(location = 7) out uint OUT_ID;

#endif