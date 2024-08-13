#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"

layout(location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec2 attribs;

layout(push_constant) uniform TraceSetting {
	int mode;
	float _padding[3];
} traceSetting;

void main()
{
  Object object           = OBJECTS.slot[gl_InstanceCustomIndexEXT];
  MaterialInfo material   = fetchMaterial(object);

  const vec3 barycentrics = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
  Vertex vert = vertexTransform(fetchVertex(object, gl_PrimitiveID, barycentrics), object.model);

  vec4 albedo = fetchDiffuse(material, vert.texCoord);  	
  vec3 normal = fetchNormal(material, vert.texCoord, vert.normal, vert.tangent);		
  float metallic = fetchMetallic(material, vert.texCoord);
  float roughness = fetchRoughness(material, vert.texCoord);

  vec3 worldPos = vert.pos; 
  float dist = length(worldPos - gl_WorldRayOriginEXT);


  if(traceSetting.mode == 0) hitValue = albedo.rgb;
  if(traceSetting.mode == 1) hitValue = (normal.rgb + 1.0f) / 2.0f;
  if(traceSetting.mode == 2) hitValue = vec3(roughness, metallic, 0.0f);

  if(traceSetting.mode == 3) hitValue = worldPos;
  if(traceSetting.mode == 4) hitValue = vec3(dist / CAMERA.near_far.y);
}
