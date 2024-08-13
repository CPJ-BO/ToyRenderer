#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/vertex.glsl"

layout (location = 0) out vec3 cubeSampleVector;
layout(location = 1) out uint OUT_ID;

void main() {

    Object object = OBJECTS.slot[gl_InstanceIndex];

    gl_Position = CAMERA.proj * CAMERA.view * object.model * vec4(POSITION, 1.0);

    cubeSampleVector = POSITION;
    cubeSampleVector.xyz *= -1.0;

    OUT_ID = gl_InstanceIndex;
}
