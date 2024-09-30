#version 460

layout (set = 0, binding = 0) uniform sampler2D samplerColor;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform CombineSetting {
    vec4 setting;
} combineSetting;

//同上采样使用的一致
vec3 Low_Filter(sampler2D tex, vec2 uv, vec2 stride, float mip)
{
	// see SIGGRAPH 2014: Advances in Real-Time Rendering
	//     "Next Generation Post-Processing in Call of Duty Advanced Warfare"
	//      Jorge Jimenez
	const float radius = 1.0;
	vec4 d = vec4(stride, -stride) * radius;

	vec3 c;
	c  = texture(tex, uv + d.zw, mip).rgb;
	c += texture(tex, uv + d.xw, mip).rgb;
	c += texture(tex, uv + d.xy, mip).rgb;
	c += texture(tex, uv + d.zy, mip).rgb;

	return c * 0.25;
}

vec3 Filter(sampler2D tex, vec2 uv, vec2 stride, float mip)
{
	// see SIGGRAPH 2014: Advances in Real-Time Rendering
	//     "Next Generation Post-Processing in Call of Duty Advanced Warfare"
	//      Jorge Jimenez
	const float radius = 1.0;
	vec4 d = vec4(stride, -stride) * radius;
	vec3 c0, c1;
	c0  = texture(tex, uv + d.zw, mip).rgb;
	c0 += texture(tex, uv + d.xw, mip).rgb;
	c0 += texture(tex, uv + d.xy, mip).rgb;
	c0 += texture(tex, uv + d.zy, mip).rgb;
	c0 += 4.0 * texture(tex, uv, mip).rgb;
	c1  = texture(tex, uv + vec2(d.z,  0.0), mip).rgb;
	c1 += texture(tex, uv + vec2(0.0,  d.w), mip).rgb;
	c1 += texture(tex, uv + vec2(d.x,  0.0), mip).rgb;
	c1 += texture(tex, uv + vec2( 0.0, d.y), mip).rgb;

	return (c0 + 2.0 * c1) * (1.0 / 16.0);
}

void main(void)
{

	vec2 tex_offset = 1.0 / textureSize(samplerColor, 0)  * combineSetting.setting.y;

	outColor = texture(samplerColor, inUV) * combineSetting.setting.x;

	//outColor = vec4(Filter(samplerColor, inUV, tex_offset, 0) * combineSetting.setting.x, 1.0);
}