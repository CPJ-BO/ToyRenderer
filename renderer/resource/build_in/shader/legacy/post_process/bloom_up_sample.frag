#version 460

layout (set = 0, binding = 0) uniform sampler2D samplerMipmap;
layout (set = 0, binding = 1) uniform sampler2D samplerLast;

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 outColor;

layout(push_constant) uniform BloomSetting {
    vec4 setting;
} bloomSetting;

//const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

const float kernel5x5[25] = float[](0.002915, 0.013064, 0.021539, 0.013064, 0.002915,
									0.013064, 0.058550, 0.096532, 0.058550, 0.013064,
									0.021539, 0.096532, 0.159155, 0.096532, 0.021539,
									0.013064, 0.058550, 0.096532, 0.058550, 0.013064,
									0.002915, 0.013064, 0.021539, 0.013064, 0.002915);

const float kernel3x3[9] = float[](	0.058550, 0.096532, 0.058550,
									0.096532, 0.159155, 0.096532,
									0.058550, 0.096532, 0.058550);

float GaussWeight2D(float x, float y, float sigma)
{
	float PI = 3.14159265358;
	float E = 2.71828182846;
	float sigma_2 = pow(sigma, 2);

	float a = -(x*x + y*y) / (2 * sigma_2);

	return pow(E, a) / (2.0 * PI * sigma_2);
}


vec3 GaussNxN(sampler2D tex, vec2 uv, int n, vec2 stride, float sigma, float mip)
{
	vec3 color = vec3(0.0);
	int r = n/2;
	float weight = 0.0;

	for(int i = -r; i <= r; i++)
	{
		for(int j = -r; j <= r; j++)
		{
			//float w = GaussWeight2D(i, j, sigma);
			//float w = kernel3x3[3 * (i+r) + (j+r)];
			float w = kernel5x5[5 * (i+r) + (j+r)];
			vec2 coord = uv + vec2(i, j) * stride;
			color += texture(tex, coord, mip).rgb * w;
			weight += w;
		}
	}

	color /= weight;

	return color;
}

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
	int index = int(bloomSetting.setting[0]);
	float stride = bloomSetting.setting[1];
	float bias = bloomSetting.setting[2];
	float intencity = bloomSetting.setting[3];

	float inIndex = index + bias;

	vec2 tex_offset_last = 1.0 / textureSize(samplerLast, 0)  * stride;
	vec2 tex_offset_mip = 1.0 / textureSize(samplerMipmap, int(inIndex))  * stride;

	//vec3 lastColor = texture(samplerLast, inUV, 0).rgb;
	vec3 lastColor = Filter(samplerLast, inUV, tex_offset_last, 0);
	vec3 mipmapColor = Filter(samplerMipmap, inUV, tex_offset_mip, inIndex);

	outColor = vec4((mipmapColor + lastColor) / 2.0 * intencity, 1.0);

	//outColor = vec4(tex_offset_last, tex_offset_mip);
	//outColor = vec4(texture(samplerLast, inUV, 0).rgb + vec3(0.01f), index);

	//outColor = vec4(mipmapColor, textureSize(samplerMipmap, int(inIndex)));
}