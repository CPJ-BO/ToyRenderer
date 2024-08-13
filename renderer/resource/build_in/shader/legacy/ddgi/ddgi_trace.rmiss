#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"

#include "ddgi_layout.glsl"

layout(push_constant) uniform TraceSetting {
	int volume_light_id;
	float _padding[3];
} traceSetting;

layout(location = 0) rayPayloadInEXT DDGIPayload payload;

void main()
{
	vec3 skyLight = fetchSkyLight(gl_WorldRayDirectionEXT);

	//payload.dist	//初始化时已经是最远距离
	payload.albedo = vec4(skyLight, 0.0f);
}