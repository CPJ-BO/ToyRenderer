#version 460

layout (set = 0, binding = 0) uniform sampler2D samplerMipmap;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform BloomSetting {
    vec4 setting;
} bloomSetting;

vec3 Box4x4(vec3 s0, vec3 s1, vec3 s2, vec3 s3) {
	return (s0 + s1 + s2 + s3) * 0.25;
}

vec3 Filter(sampler2D tex, vec2 uv, vec2 stride, int mip)
{
	// see SIGGRAPH 2014: Advances in Real-Time Rendering
	//     "Next Generation Post-Processing in Call of Duty Advanced Warfare"
	//      Jorge Jimenez
	vec3 c = texture(tex, uv, mip).rgb;

	// The offsets below are in "source" texture space
	vec3 lt  = texture(tex, uv + ivec2(-1, -1) * stride, mip).rgb;
	vec3 rt  = texture(tex, uv + ivec2( 1, -1) * stride, mip).rgb;
	vec3 rb  = texture(tex, uv + ivec2( 1,  1) * stride, mip).rgb;
	vec3 lb  = texture(tex, uv + ivec2(-1,  1) * stride, mip).rgb;

	vec3 lt2 = texture(tex, uv + ivec2(-2, -2) * stride, mip).rgb;
	vec3 rt2 = texture(tex, uv + ivec2( 2, -2) * stride, mip).rgb;
	vec3 rb2 = texture(tex, uv + ivec2( 2,  2) * stride, mip).rgb;
	vec3 lb2 = texture(tex, uv + ivec2(-2,  2) * stride, mip).rgb;

	vec3 l   = texture(tex, uv + ivec2(-2,  0) * stride, mip).rgb;
	vec3 t   = texture(tex, uv + ivec2( 0, -2) * stride, mip).rgb;
	vec3 r   = texture(tex, uv + ivec2( 2,  0) * stride, mip).rgb;
	vec3 b   = texture(tex, uv + ivec2( 0,  2) * stride, mip).rgb;

	// five h4x4 boxes
	vec3 c0, c1;

	// common case
	c0  = Box4x4(lt, rt, rb, lb);
	c1  = Box4x4(c, l, t, lt2);
	c1 += Box4x4(c, r, t, rt2);
	c1 += Box4x4(c, r, b, rb2);
	c1 += Box4x4(c, l, b, lb2);

	// weighted average of the five boxes
	return c0 * 0.5 + c1 * 0.125;

}


void main(void)
{
	int index = int(bloomSetting.setting[0]);
	float stride = bloomSetting.setting[1];
	float bias = bloomSetting.setting[2];
	float intencity = bloomSetting.setting[3];

	float inIndex = index + bias;

	vec2 tex_offset = 1.0 / textureSize(samplerMipmap, 0) * stride;

	vec3 mipmapColor = Filter(samplerMipmap, inUV, tex_offset, 0);

	//outColor = vec4(vec3(inIndex / 10.0), 1.0);
	outColor = vec4(mipmapColor, 1.0);

	//outColor = vec4(tex_offset, 0.0, 1.0);
}