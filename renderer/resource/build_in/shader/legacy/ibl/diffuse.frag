// Generates an irradiance cube from an environment map using convolution

// https://zhuanlan.zhihu.com/p/66518450

#version 460

layout (location = 0) in vec3 inFront;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform samplerCube samplerEnv;

layout(push_constant) uniform PushConsts {
	layout (offset = 32) float deltaPhi;
	layout (offset = 36) float deltaTheta;
} consts;

#define PI 3.1415926535

void main()
{
	vec3 N = normalize(inFront);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up = cross(N, right);

	const float TWO_PI = PI * 2.0;
	const float HALF_PI = PI * 0.5;

	vec3 color = vec3(0.0);
	uint sampleCount = 0u;
	for (float phi = 0.0; phi < TWO_PI; phi += consts.deltaPhi) {
		for (float theta = 0.0; theta < HALF_PI; theta += consts.deltaTheta) {
			vec3 tempVec = cos(phi) * right + sin(phi) * up;
			vec3 sampleVector = cos(theta) * N + sin(theta) * tempVec;
			color += texture(samplerEnv, sampleVector).rgb * cos(theta) * sin(theta);
			sampleCount++;
		}
	}
	
	outColor = vec4(PI * color / float(sampleCount), 1.0);

	//outColor = vec4(N, 1.0);
	
	//outColor = vec4(1.0);
}
