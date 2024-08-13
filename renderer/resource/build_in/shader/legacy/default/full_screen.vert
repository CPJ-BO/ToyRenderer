#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) out vec2 outUV;

// out gl_PerVertex
// {
// 	vec4 gl_Position;
// };

void main() 
{
	outUV 				= vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2); 	//[0, 0]	~	[2, 2]
	vec2 offset 		= outUV * 2.0f - 1.0f;									//[-1, -1]	~	[3, 3], [1, 1]之外被裁剪，总共只有一个三角形

	gl_Position 		= vec4(outUV * 2.0f - 1.0f, 0.0f, 1.0f);
}
