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
hitAttributeEXT vec2 attribs;

void main()
{
  Object object           = OBJECTS.slot[gl_InstanceCustomIndexEXT];
  MaterialInfo material   = fetchMaterial(object);

  const vec3 barycentrics = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
  Vertex vert       = vertexTransform(fetchVertex(object, gl_PrimitiveID, barycentrics), object.model);


  payload.dist      = length(vert.pos - gl_WorldRayOriginEXT); 
  payload.normal    = vec4(fetchNormal(material, vert.texCoord, vert.normal, vert.tangent), 0.0f);
  payload.albedo    = vec4(fetchDiffuse(material, vert.texCoord).rgb, 0.0f);  	
  payload.emission  = vec4(fetchEmission(material), 0.0f);

  payload.normal.w  = fetchRoughness(material, vert.texCoord);  //粗糙度
  payload.albedo.w  = fetchMetallic(material, vert.texCoord);   //金属度
}

