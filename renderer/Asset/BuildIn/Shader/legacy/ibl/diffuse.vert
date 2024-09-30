#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

//#include "../include/common.glsl"
//#include "../include/vertex.glsl"

layout(push_constant) uniform PushConsts {
	layout (offset = 0) 	vec4 front;
	layout (offset = 16) 	vec4 up;
} pushConsts;

layout (location = 0) out vec3 outFront;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() 
{
	vec2 outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);	//铺屏四边形，坐标左上[0,0] 右下[1,1],vulkan ndc的y是朝下的
	outUV = outUV * 2.0f - 1.0f;										//左上[-1,-1] 右下[1,1]

	gl_Position = vec4(outUV, 0.0f, 1.0f);

	vec3 right = cross(pushConsts.front.xyz, pushConsts.up.xyz);

	outFront = normalize(pushConsts.front.xyz) + outUV.x * normalize(right.xyz) + outUV.y * normalize(pushConsts.up.xyz);
	outFront.z *= -1;	//为什么是反的？妈妈生的
}



