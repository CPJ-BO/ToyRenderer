#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"

layout(push_constant) uniform DeferredSetting {
	float mode;
} deferredSetting;

// layout (input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput samplerVelocity;
// layout (input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput samplerNormal;
// layout (input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput samplerAlbedo;
layout (set = 1, binding = 0) uniform sampler2D samplerVelocity;
layout (set = 1, binding = 1) uniform sampler2D samplerNormal;
layout (set = 1, binding = 2) uniform sampler2D samplerAlbedo;
layout (set = 1, binding = 3) uniform sampler2D samplerEmission;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 color;

void main() 
{  
    //材质参数////////////////////////////////////////////////////////////////////////////////////////////////////////                       

    // Get G-Buffer values
    // vec4 velocity_data = subpassLoad(samplerVelocity).rgba; 
    // vec4 normal_data = subpassLoad(samplerNormal).rgba;
    // vec4 albedo_data = subpassLoad(samplerAlbedo).rgba;
    vec4 velocity_data = texture(samplerVelocity, inUV).rgba; 
    vec4 normal_data = texture(samplerNormal, inUV).rgba; 
    vec4 albedo_data = texture(samplerAlbedo, inUV).rgba; 
    vec4 emission_data = texture(samplerEmission, inUV).rgba; 
        
    vec3 pos        = depthToWorld(inUV).xyz;   //世界位置
    vec3 albedo     = albedo_data.rgb;          //漫反射颜色
    vec3 normal     = normal_data.rgb;          //法线
    vec3 velocity   = velocity_data.rgb;        //速度
    vec3 emission   = emission_data.rgb;        //自发光

    //normal = fetchNormal(inUV);               //等价的方法
    //velocity = fetchVelocity(inUV);

    float ao = texture(AO_SAMPLER, inUV).r; //AO

    float roughness = velocity_data.a;       //粗糙度
    float metallic = normal_data.a;         //金属度
    float enabled = albedo_data.a;          //启用

    vec3 N = normalize(normal);    //法线

    float a2 = pow(roughness, 4) ;                              //粗糙度


    vec3 F0 = vec3(0.04f);                                  //菲涅尔反射率，表征对不同波长光的折射差异（也就是镜面反射颜色）
    F0 = mix(F0, albedo.rgb, metallic);
  	//float F90 = clamp(50.0 * F0.r, 0.0, 1.0);

    //光照计算相关向量////////////////////////////////////////////////////////////////////////////////////////////////////////

	vec3 worldPos = pos;
	vec3 viewPos = (CAMERA.view * vec4(pos, 1.0)).xyz;

    vec3 viewVec = CAMERA.pos.xyz - pos;		
	vec3 V = normalize(viewVec);	//视线
    vec3 R = normalize(reflect(-V, N));     //反射

    //直接光照计算////////////////////////////////////////////////////////////////////////////////////////////////////////

    DirectLightDebug debug0;
    vec3 dirctLightColor = 
        directLight(
        albedo, F0, roughness, metallic,
        vec4(worldPos, 1.0f), N, V,
        false, false,
        debug0);

    //间接光照计算////////////////////////////////////////////////////////////////////////////////////////////////////////

    IndirectLightDebug debug1;
    vec3 indirctLightColor = 
        indirectLight(
        albedo, F0, roughness, metallic,
        vec4(worldPos, 1.0f), N, V,
        debug1);

    indirctLightColor *= ao;

	//最终颜色输出////////////////////////////////////////////////////////////////////////////////////////////////////////
    vec4 outColor = vec4(dirctLightColor + indirctLightColor + emission, 1.0);	

    if(deferredSetting.mode == 1)   outColor = vec4((normalize(normal) + vec3(1.0)) / 2, 1.0);	       //Normal
	if(deferredSetting.mode == 2)   outColor = vec4(roughness, metallic, 0.0, 1.0);					    //Roughness/Metallic
    if(deferredSetting.mode == 3)   outColor = vec4(worldPos, 1.0);                                     //World pos
    if(deferredSetting.mode == 4)   outColor = vec4(vec3(debug0.shadow), 1.0);	                        //Shadow
	if(deferredSetting.mode == 5)   outColor = vec4(vec3(ao), 1.0);								        //AO
    if(deferredSetting.mode == 6)  
    {
        int cascadeIndex = 0;
        for(int i = 0; i <= 3; i++) {
            //if((CAMERA.view * inPos).z > 0) {	
            if(-viewPos.z > LIGHT.directionalLights[i].depth.x) {	
                cascadeIndex = i + 1;
            }
        }

        if(cascadeIndex == 0)       outColor = vec4(1.0, 0.0, 0.0, 1.0);	
        if(cascadeIndex == 1)       outColor = vec4(0.0, 1.0, 0.0, 1.0);	
        if(cascadeIndex == 2)       outColor = vec4(0.0, 0.0, 1.0, 1.0);	
        if(cascadeIndex == 3)       outColor = vec4(0.0, 1.0, 1.0, 1.0);	
        	
    }	
    if(deferredSetting.mode == 7)   outColor = vec4(vec3(debug0.clusterLightCount), 1.0);
    if(deferredSetting.mode == 8)   outColor = vec4(debug0.clusterOffset.xy, 0.0, 1.0);	
    if(deferredSetting.mode == 9)   outColor = vec4(vec3(debug0.clusterOffset.z), 1.0);	
    if(deferredSetting.mode == 10)  outColor = vec4(indirctLightColor, 1.0f);   
    if(deferredSetting.mode == 11)  outColor = vec4(debug1.indirectIrradiance, 1.0f);   
    if(deferredSetting.mode == 12)  outColor = vec4(dirctLightColor + emission, 1.0);   

    color = outColor;
}