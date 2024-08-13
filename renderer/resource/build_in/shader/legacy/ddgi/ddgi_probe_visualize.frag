#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"

#include "ddgi_layout.glsl"

layout(location = 0) in vec4 IN_POSITION;
layout(location = 1) in vec3 IN_NORMAL;
layout(location = 2) in flat int IN_ID;

layout(push_constant) uniform VisualizeSetting {
	int volume_light_id;
    float probe_scale;
	float visualize_mode;
	float _padding;
} visualizeSetting;

layout (location = 0) out vec4 color;

void main() 
{
	VolumeLight volumeLight = LIGHT.volumeLights[visualizeSetting.volume_light_id];

	if(visualizeSetting.visualize_mode == 0)
	{
		// 辐照度
		vec2 probe_coord = fetchProbeAbsTexCoord(
			normalize(IN_NORMAL), 
			IN_ID, 
			volumeLight.setting.irradiance_texture_width, 
			volumeLight.setting.irradiance_texture_height, 
			DDGI_IRRADIANCE_PROBE_SIZE); 

		color = texture(VOLUME_LIGHT_IRRADIANCE_SAMPLERS[visualizeSetting.volume_light_id], probe_coord);
	}
	else
	{
		// 深度
		vec2 probe_coord = fetchProbeAbsTexCoord(
		normalize(IN_NORMAL), 
		IN_ID, 
		volumeLight.setting.depth_texture_width, 
		volumeLight.setting.depth_texture_height, 
		DDGI_DEPTH_PROBE_SIZE); 

		color = vec4(texture(VOLUME_LIGHT_DEPTH_SAMPLERS[visualizeSetting.volume_light_id], probe_coord).r / volumeLight.setting.max_probe_distance);
	}
}