#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/vertex.glsl"

layout(push_constant) uniform GizmosSetting {
    vec4 color;
    mat4 model;
} gizmosSetting;

layout(location = 0) out vec4 OUT_COLOR;

void main() {
    
    gl_Position = CAMERA.proj * CAMERA.view * gizmosSetting.model * vec4(POSITION, 1.0);

    OUT_COLOR = gizmosSetting.color;
}





