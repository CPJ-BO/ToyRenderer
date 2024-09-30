#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_frag.glsl"

layout(location = 0) out vec4 color;

layout(location = 0) in vec3 IN_NORMAL;
layout(location = 1) in vec2 IN_TEXCOORD;
layout(location = 2) in flat int IN_ID;

void main() 
{	
    Object object           = OBJECTS.slot[IN_ID];
    MaterialInfo material   = fetchMaterial(object);     
    vec4 albedo             = fetchDiffuse(material, IN_TEXCOORD);  	

    //alphaClip(material, albedo.a);    

	//color = vec4((normalize(IN_NORMAL) + vec3(1.0)) / 2.0, 1.0);
    color = vec4(IN_NORMAL, 1.0);
}