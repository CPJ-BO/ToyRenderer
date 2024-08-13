
#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/vertex.glsl"

#include "ddgi_layout.glsl"

layout(location = 0) out vec4 OUT_POSITION;
layout(location = 1) out vec3 OUT_NORMAL;
layout(location = 2) out int OUT_ID;

layout(push_constant) uniform VisualizeSetting {
	int volume_light_id;
    float probe_scale;
	float _padding[2];
} visualizeSetting;

out gl_PerVertex 
{
    vec4 gl_Position;   
};


void main() {

    VolumeLight volumeLight = LIGHT.volumeLights[visualizeSetting.volume_light_id];
    
    ivec3 grid_coord        = fetchProbeCoord(volumeLight.setting, gl_InstanceIndex);
    vec3 probe_position     = fetchProbeWorldPos(volumeLight.setting, grid_coord);
    vec4 worldPos           = vec4(POSITION * visualizeSetting.probe_scale + probe_position, 1.0f);

    OUT_POSITION            = worldPos;
    OUT_ID                  = gl_InstanceIndex;
    OUT_NORMAL              = NORMAL;

    gl_Position = CAMERA.proj * CAMERA.view * worldPos;
}








