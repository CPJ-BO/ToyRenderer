#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_frag.glsl"
#include "../include/frag.glsl"

layout (location = 0) out vec4 outVelocity;	//速度buffer
layout (location = 1) out vec4 outNormal;	//法线buffer
layout (location = 2) out vec4 outAlbedo;	//漫反射颜色buffer
layout (location = 3) out vec4 outEmission;	//自发光颜色buffer


void main() 
{
	Object object           = OBJECTS.slot[ID];
    MaterialInfo material   = fetchMaterial(object); 

  	outVelocity = writeVelocity(POSITION, PREV_POSITION);

	//材质参数////////////////////////////////////////////////////////////////////////////////////////////////////////

	vec4 albedo         	= fetchDiffuse(material, TEXCOORD);  
	//albedo					= vec4(COLOR, 1.0);		
    vec3 normal         	= fetchNormal(material, TEXCOORD, NORMAL, TANGENT);			                               
    float roughness     	= fetchRoughness(material, TEXCOORD);                                             
    float metallic      	= fetchMetallic(material, TEXCOORD);    
    vec3 emission       	= fetchEmission(material);    						

	//alphaClip(material, albedo.a);    

	//输出////////////////////////////////////////////////////////////////////////////////////////////////////////

	//outVelocity = vec4(POSITION.xyz, 1.0);	//不需要写位置了，由深度重建
	outNormal = vec4(normal, 0.0);
	outAlbedo = albedo;
	outEmission = vec4(emission, 0.0);
	
	float enabled = 2;		

	outVelocity.a = roughness;		//粗糙度
	outNormal.a = metallic;		//金属度
	outAlbedo.a = enabled;		//启用

}