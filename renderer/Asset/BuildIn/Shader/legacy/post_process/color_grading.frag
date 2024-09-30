#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"

layout (set = 1, binding = 0) uniform sampler2D inputColor;
layout (set = 1, binding = 1) uniform sampler2D colorLUT;

layout(push_constant) uniform ColorGradingSetting {
    float exposure;
	float mode;
	float lut;
	float saturation;
	float contrast;
} setting;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;


//CryEngine 2
vec3 CEToneMapping(vec3 color, float adapted_lum) 
{
    return vec3(1.0) - exp(-adapted_lum * color);
}


//Uncharted 2 - filmic tone mapping
vec3 F(vec3 x)
{
	const float A = 0.22f;
	const float B = 0.30f;
	const float C = 0.10f;
	const float D = 0.20f;
	const float E = 0.01f;
	const float F = 0.30f;
 
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 Uncharted2ToneMapping(vec3 color, float adapted_lum)
{
	const float WHITE = 11.2f;
	return F(1.6f * adapted_lum * color) / F(vec3(WHITE));
}

//ACES
vec3 ACESToneMapping(vec3 color, float adapted_lum)
{
	const float A = 2.51f;
	const float B = 0.03f;
	const float C = 2.43f;
	const float D = 0.59f;
	const float E = 0.14f;

	color *= adapted_lum;
	return (color * (A * color + B)) / (color * (C * color + D) + E);
}

//饱和度
vec3 SaturationColor(vec3 color, float saturation)
{
     vec3 luminanceWeighting = vec3(0.2125, 0.7154, 0.0721);
     float luminance = dot(color, luminanceWeighting);

     //灰度校准向量
     vec3 greyScaleColor = vec3(luminance);

     return mix(greyScaleColor, color, saturation);
}

//对比度
vec3 ContrastColor(vec3 color, float contrast)
{
    return (color - vec3(0.5)) * contrast + vec3(0.5);
}

void main() 
{
	vec3 color;

	color = texture(inputColor, inUV).rgb;
	//color = pow(color, vec3(1.0/2.2));   

	float finalExposure = setting.exposure / EXPOSURE.luminance;	//使得整个画面的输出光照接近setting.exposure，例如0.5
	
	if(setting.mode == 0) color = CEToneMapping(color, finalExposure); 
	if(setting.mode == 1) color = Uncharted2ToneMapping(color, finalExposure); 
	if(setting.mode == 2) color = ACESToneMapping(color, finalExposure); 

	//color = pow(color, vec3(2.2)); 
	//color = pow(color, vec3(1.0/2.2)); 


	//LUT是2D纹理，要采样两次手动做插值  
	//
	if(setting.lut > 0.0f)
	{
		color 				= clamp(color, vec3(0.08f), vec3(0.92f));	//采样在边缘还有问题 picolo教程说是mipmap的问题？

		ivec2 lut_tex_size 	= textureSize(colorLUT, 0);

		float b 			= color.b * lut_tex_size.y;	//b通道计算offset
		float b_floor 		= floor(b);
		float b_ceil 		= ceil(b);

		vec3 color_floor 	= texture(colorLUT, vec2((b_floor + color.r) / lut_tex_size.y, color.g)).rgb;
		vec3 color_ceil 	= texture(colorLUT, vec2((b_ceil + color.r) / lut_tex_size.y, color.g)).rgb;
		color_floor 		= pow(color_floor, vec3(1.0/2.2));   
		color_ceil 			= pow(color_ceil, vec3(1.0/2.2));   

		color = mix(color_floor, color_ceil, b - b_floor);
	}

	color = SaturationColor(color, setting.saturation);
	color = ContrastColor(color, setting.contrast);

	outColor = vec4(color, 1.0f);
}