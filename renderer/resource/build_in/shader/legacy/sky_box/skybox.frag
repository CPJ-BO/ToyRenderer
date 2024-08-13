#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"

layout (location = 0) in vec3 cubeSampleVector;
layout(location = 1) in flat uint ID;

layout(location = 0) out vec4 outColor;

void main(){

    // Object object           = OBJECTS.slot[ID];
    // MaterialInfo material   = fetchMaterial(object); 

    // vec3 sky = fetchMaterialTexCube(material, 0, cubeSampleVector).rgb;
    // float intencity = fetchMaterialFloat(material, 0);
    // vec3 color = pow(sky, vec3(1.0/2.2));
    // color *= intencity;

    vec3 skyLight = fetchSkyLight(cubeSampleVector);    //和以上代码实现相同

    outColor = vec4(skyLight, 1.0);
}