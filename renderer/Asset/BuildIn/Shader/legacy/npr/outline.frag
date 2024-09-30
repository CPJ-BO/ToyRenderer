#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_frag.glsl"


layout(location = 0) in vec4 POSITION;
layout(location = 1) in vec4 PREV_POSITION;
layout(location = 2) in flat uint ID;

layout(location = 0) out vec4 color;
layout (location = 1) out vec4 outVelocity;		//速度buffer

void main() 
{	
    //TODO 这个着色器还有未知bug，启动时有可能崩溃？

    Object object           = OBJECTS.slot[ID];
    MaterialInfo material   = fetchMaterial(object); 

    outVelocity = writeVelocity(POSITION, PREV_POSITION);

    vec4 outlineColor = fetchMaterialColor(material, 0);                                      //colors[0] Outline color

	color = vec4(outlineColor.rgb, 1.0);
}