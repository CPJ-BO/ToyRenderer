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

layout(location = 8) in flat uvec2 IN_CLUSTER_DATA;

const vec3 colors[7] = {
	vec3(1.0f, 0.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 0.0f, 1.0f),
	vec3(1.0f, 1.0f, 0.0f),
	vec3(1.0f, 0.0f, 1.0f),
	vec3(0.0f, 1.0f, 1.0f),
	vec3(1.0f, 1.0f, 1.0f)
};

void main() 
{
	Object object           = OBJECTS.slot[ID];
    MaterialInfo material   = fetchMaterial(object); 

  	outVelocity = writeVelocity(POSITION, PREV_POSITION);

	//材质参数////////////////////////////////////////////////////////////////////////////////////////////////////////

	vec4 albedo         	= fetchDiffuse(material, TEXCOORD);  
	//vec4 albedo					= vec4(COLOR, 1.0);		
    vec3 normal         	= fetchNormal(material, TEXCOORD, NORMAL, TANGENT);			                               
    float roughness     	= fetchRoughness(material, TEXCOORD);                                             
    float metallic      	= fetchMetallic(material, TEXCOORD);    
    vec3 emission       	= fetchEmission(material);    						

	//alphaClip(material, albedo.a);    

	//输出////////////////////////////////////////////////////////////////////////////////////////////////////////

	//outVelocity = vec4(POSITION.xyz, 1.0);	//不需要写位置了,由深度重建
	outNormal = vec4(normal, 0.0);
	//outAlbedo = albedo;
	outEmission = vec4(emission, 0.0);
	
	float enabled = 2;		

	outVelocity.a = roughness;	//粗糙度
	outNormal.a = metallic;		//金属度
	outAlbedo.a = enabled;		//启用




	// float cluster = float(IN_CLUSTER_ID);

    // float r =  random(cluster + 0.1f);
    // float g =  random(cluster + 0.2f);
    // float b =  random(cluster + 0.3f);
    // outAlbedo.xyz = vec3(r, g, b);

	if(GLOBAL_SETTING.clusterInspectMode == 0)	outAlbedo.xyz = albedo.xyz;
	if(GLOBAL_SETTING.clusterInspectMode == 1)	outAlbedo.xyz = colors[IN_CLUSTER_DATA[0] % 7];
	if(GLOBAL_SETTING.clusterInspectMode == 2)	outAlbedo.xyz = colors[IN_CLUSTER_DATA[1] % 7];
}

