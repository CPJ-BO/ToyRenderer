#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_frag.glsl"
#include "../include/frag.glsl"

#define AMBIENT 0.1

layout(location = 0) out vec4 color;

void main() 
{
	//材质参数////////////////////////////////////////////////////////////////////////////////////////////////////////

    Object object           = OBJECTS.slot[ID];
    MaterialInfo material   = fetchMaterial(object);      
                    
    vec4 albedo             = fetchDiffuse(material, TEXCOORD);  		
    vec3 normal             = fetchNormal(material, TEXCOORD, NORMAL, TANGENT);			                               
    // float roughness         = fetchRoughness(material, TEXCOORD);                                             
    // float metallic          = fetchMetallic(material, TEXCOORD);  
    // float ao                = fetchAO(material, TEXCOORD);   
    // vec3 emission           = fetchEmission(material);   

    //alphaClip(material, albedo.a);                                    

	//光照计算相关向量////////////////////////////////////////////////////////////////////////////////////////////////////////

    vec4 outColor; 
    outColor = albedo;




    color = outColor;

}