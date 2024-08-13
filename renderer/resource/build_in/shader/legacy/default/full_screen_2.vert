#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outVec;

out gl_PerVertex
{
	vec4 gl_Position;
};

// 已弃用。使用视锥的四个斜边向量复原世界坐标

void main() 
{
	outUV 				= vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2); 	//[0, 0]	~	[2, 2]
	vec2 offset 		= outUV * 2.0f - 1.0f;									//[-1, -1]	~	[3, 3], [1, 1]之外被裁剪，总共只有一个三角形

	//世界空间的向量
	vec3 up 			= CAMERA.up.xyz;			
	vec3 right 			= CAMERA.right.xyz;
	vec3 front 			= CAMERA.front.xyz;

	//计算视锥的四个斜边向量
	float halfHeight 	= CAMERA.near_far.x * tan(CAMERA.fov_aspect.x / 2.0);	//入参是弧度！！！！！！
	float aspect 		= CAMERA.fov_aspect.y;

	vec3 toTop 			= normalize(up) * halfHeight;
	vec3 toRight 		= normalize(right) * halfHeight * aspect;
	vec3 toNear 		= normalize(front) * CAMERA.near_far.x;

	outVec 				= toNear + toTop * -offset.y + toRight * offset.x;

	gl_Position 		= vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}


// vec3 fetchDepthToWorld(vec3 frustumVec, vec2 coord)
// {
//     ivec2 texDim        = textureSize(DEPTH_SAMPLER, 0); 

// 	float depth         = texture(DEPTH_SAMPLER, coord).r;
// 	float linearDepth   = LinearEyeDepth(depth, CAMERA.near_far.x, CAMERA.near_far.y);

// 	vec3 offsetPos      = (linearDepth / CAMERA.near_far.x) * frustumVec;					//根据世界空间的视锥向量算出来的世界空间相对相机的偏移
// 	vec3 worldPos       = CAMERA.pos.xyz + offsetPos;										//世界空间坐标

//     return worldPos;
// }