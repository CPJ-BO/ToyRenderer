#version 460

layout (set = 0, binding = 0) uniform sampler2D samplerColor;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outClampedColor;
//layout (location = 1) out vec4 outColor;

void main(void)
{
	vec4 color = texture(samplerColor, inUV).rgba;
	/*
	outColor = color;
	outClampedColor = vec4(0.0);

	if(color.r > 1 || color.g > 1 || color.b > 1) 
	{
		outClampedColor = color;
	}
	*/

	if(color.r <= 1 && color.g <= 1 && color.b <= 1) discard;
	if(color.r <= 0 || color.g <= 0 || color.b <= 0) discard;
	outClampedColor = color;
	//outClampedColor = vec4(max(vec3(0.0), color.rgb - vec3(1.0)), 1.0);
}