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

    //alphaClip(material, albedo.a);                                    

	//光照计算相关向量////////////////////////////////////////////////////////////////////////////////////////////////////////

	vec3 worldPos = POSITION.xyz;
	vec3 viewPos = (CAMERA.view * POSITION).xyz;

    vec3 N = normalize(normal);	            //法线
    vec3 V = normalize(VIEW_VEC);	        //视线
    vec3 R = normalize(reflect(-V, N));     //反射

    float a2 = pow(roughness, 4) ;                                              //粗糙度

    vec3 F0 = vec3(0.04f);                                                      //菲涅尔反射率，表征对不同波长光的折射差异（也就是镜面反射颜色）
    F0 = mix(F0, albedo.rgb, metallic);

    //直接光照计算////////////////////////////////////////////////////////////////////////////////////////////////////////

    vec3 dirctLightColor = 
        directLight(
        albedo.xyz, F0, roughness, metallic,
        vec4(worldPos, 1.0f), N, V,
        true, true);

    //间接光照计算////////////////////////////////////////////////////////////////////////////////////////////////////////

    vec3 indirctLightColor = 
        indirectLight(
        albedo.xyz, F0, roughness, metallic,
        vec4(worldPos, 1.0f), N, V);

    indirctLightColor *= ao;

	//最终颜色输出////////////////////////////////////////////////////////////////////////////////////////////////////////

    vec3 ambientColor = vec3(0.0);
    {
        float NoV = clamp(dot(N, V), 0.00001, 0.99999);

        vec3 kS = FresnelSchlickRoughness(NoV, F0, roughness); 
        vec3 kD = 1.0 - kS;

        //漫反射IBL
        vec3 irradiance = texture(DIFFUSE_IBL_SAMPLER, N).rgb;
        vec3 diffuse    = irradiance * albedo.rgb;
        vec3 diffuseColor = kD * diffuse; //漫反射环境光照得到的颜色

        //镜面反射IBL
        vec2 brdf = texture(BRDF_LUT_SAMPLER, vec2(NoV, roughness)).rg;
        brdf = pow(brdf, vec2(1.0/2.2));      //gamma矫正
        vec3 reflection = PreFilteredReflection(R, roughness, SPECULAR_IBL_SAMPLER).rgb;	
	    vec3 specularColor = reflection * (kS * brdf.x + brdf.y);

        ambientColor = diffuseColor + specularColor;
        ambientColor *= ao;
    }

    vec4 outColor; 
    outColor = vec4(dirctLightColor + indirctLightColor + ambientColor, 1.0);	
	outColor += vec4(emission, 0.0);;


    color = outColor;
}