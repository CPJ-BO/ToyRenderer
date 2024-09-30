#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../common/common.glsl"

layout(location = 0) in vec3 IN_CUBE_SAMPLE_VECTOR;
layout(location = 1) in flat uint IN_ID;

layout (location = 0) out vec4 OUT_COLOR;

void main()
{
    Material material = FetchMaterial(IN_ID);

    vec3 sky = FetchMaterialTexCube(material, 0, IN_CUBE_SAMPLE_VECTOR).rgb;
    //float intencity = FetchMaterialFloat(material, 0);
    float intencity = 1.0f;
    vec3 color = pow(sky, vec3(1.0/2.2));
    color *= intencity;

    OUT_COLOR = vec4(color, 1.0);
    //OUT_COLOR = vec4(normalize(IN_CUBE_SAMPLE_VECTOR), 1.0);
}