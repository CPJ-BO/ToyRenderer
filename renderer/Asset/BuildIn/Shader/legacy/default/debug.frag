#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_frag.glsl"
#include "../include/frag.glsl"

#define AMBIENT 0.1

layout (location = 0) out vec4 color;
layout (location = 1) out vec4 outVelocity;		//速度buffer

void main() 
{
    Object object           = OBJECTS.slot[ID];
    MaterialInfo material   = fetchMaterial(object);  

    outVelocity = writeVelocity(POSITION, PREV_POSITION);

	//材质参数////////////////////////////////////////////////////////////////////////////////////////////////////////

    vec4 albedo             = fetchDiffuse(material, TEXCOORD);  		
    vec3 normal             = fetchNormal(material, TEXCOORD, NORMAL, TANGENT);			                               
    float roughness         = fetchRoughness(material, TEXCOORD);                                             
    float metallic          = fetchMetallic(material, TEXCOORD);  
    float ao                = fetchAO(material, TEXCOORD);   
    vec3 emission           = fetchEmission(material);   



    vec4 outColor = object.debugData;
    outColor = vec4(vec3(ao), 1.0f);

    color = outColor;

}