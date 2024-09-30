#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_frag.glsl"

layout(location = 0) in vec2 IN_TEXCOORD;
layout(location = 1) in flat int IN_ID;

//layout(location = 0) out vec4 color;

void main() 
{	
    Object object           = OBJECTS.slot[IN_ID];
    MaterialInfo material   = fetchMaterial(object); 
    vec4 albedo             = fetchDiffuse(material, IN_TEXCOORD);  	
    //alphaClip(material, albedo.a);    
}