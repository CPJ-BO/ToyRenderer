#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"

#define SAMPLE_SIZE 32
#define NOISE_SIZE 16
#define NOISE_WIDTH 4

layout(set = 1, binding = 0) uniform samples { 
    vec4 sampleVec[SAMPLE_SIZE];
	vec4 noises[NOISE_SIZE];
} SAMPLES;

layout(location = 0) out float color;

layout (location = 0) in vec2 inUV;

layout(push_constant) uniform AoSetting {
	vec4 radius_remap;
} aoSetting;

void main() 
{	
	ivec2 texDim = textureSize(DEPTH_SAMPLER, 0); 

	float depth = texture(DEPTH_SAMPLER, inUV).r;
	float linearDepth = LinearEyeDepth(depth, CAMERA.near_far.x, CAMERA.near_far.y);

	vec3 worldPos = depthToWorld(inUV).xyz;

	//vec3 normal = normalize(texture(NORMAL_SAMPLER, inUV).xyz * 2.0 - 1.0);		//世界空间法线
	vec3 normal = normalize(texture(NORMAL_SAMPLER, inUV).xyz);		

	// 查表获取随机抖动的噪声向量
	ivec2 noiseUV = ivec2(texDim * inUV) % NOISE_WIDTH;
	vec3 randomVec = SAMPLES.noises[noiseUV.x * NOISE_WIDTH + noiseUV.y].xyz;

	// TBN矩阵
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	// 搜索范围
	float radius = aoSetting.radius_remap[0];
	float remap = aoSetting.radius_remap[1];

	// Calculate occlusion value
	float occlusion = 0.0;
	for(int i = 0; i < SAMPLE_SIZE; ++i)
	{
		// 获取样本位置
		vec3 samplePos = TBN * SAMPLES.sampleVec[i].xyz; 	// 切线->世界空间
		samplePos = worldPos + samplePos * radius; 

		vec4 offset = vec4(samplePos, 1.0);
		offset = CAMERA.proj * CAMERA.view * offset; 			// 世界->裁剪空间
		offset.xyz /= offset.w; 							// 透视除法
		offset.xy = offset.xy * 0.5 + 0.5; 					// 变换到0.0 - 1.0的值域

		float sampleDepth = texture(DEPTH_SAMPLER, offset.xy).r;
		float linearSampleDepth = LinearEyeDepth(sampleDepth, CAMERA.near_far.x, CAMERA.near_far.y);
		
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(linearDepth - linearSampleDepth));
		occlusion += (sampleDepth < offset.z ? 1.0 : 0.0) * rangeCheck;   

		//occlusion += (sampleDepth < offset.z ? 1.0 : 0.0);
	}
	occlusion = 1.0 - (occlusion / float(SAMPLE_SIZE));
	occlusion = smoothstep(0.0f, 1.0f, min(1.0, occlusion / remap));    //重映射

	//color = depth;
	//color = viewPos.x;
	//color = occlusion;
	//color = worldPos.x / 50.0;
	color = occlusion;
	//color = randomVec.x;
}