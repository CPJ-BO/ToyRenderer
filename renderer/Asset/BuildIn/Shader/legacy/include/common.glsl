#ifndef COMMON_GLSL
#define COMMON_GLSL

#extension GL_EXT_nonuniform_qualifier : enable

#include "intersection.glsl"
#include "random.glsl"
#include "brdf.glsl"


#define ENABLE_RAY_TRACING true

#if (ENABLE_RAY_TRACING == true)
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable
layout(set = 0, binding = 2) uniform accelerationStructureEXT TLAS;
#endif

#define FRAMES_IN_FLIGHT 3

#define WINDOW_WIDTH 2048                          
#define WINDOW_HEIGHT 1152                          

#define HALF_WINDOW_WIDTH (WINDOW_WIDTH / 2)
#define HALF_WINDOW_HEIGHT (WINDOW_HEIGHT / 2)

#define DDGI_IRRADIANCE_PROBE_SIZE 8              
#define DDGI_DEPTH_PROBE_SIZE 16                 

#define HALF_SIZE_SSSR false

#define VOLUMETRIC_FOG_SIZE_X 320                 
#define VOLUMETRIC_FOG_SIZE_Y 180
#define VOLUMETRIC_FOG_SIZE_Z 128

#define MAX_LIGHTS_PER_CLUSTER 8                  
#define TILE_BLOCK_PIX_SIZE 64                   
#define TILE_DEPTH_SIZE 128                       
//#define LIGHT_CLUSTER_NUM   (WINDOW_WIDTH / TILE_BLOCK_PIX_SIZE) * (WINDOW_HEIGHT / TILE_BLOCK_PIX_SIZE) * TILE_DEPTH_SIZE       
#define LIGHT_CLUSTER_NUM 18432 //计算好的结果

#define MAX_PER_FRAME_RESOURCE_SIZE 10240

#define CLUSTER_TRIANGLE_SIZE 128                   
#define CLUSTER_GROUP_SIZE 32                      
#define MAX_GLOBAL_CLUSTER_SIZE 10240
#define MAX_GLOBAL_CLUSTER_GROUP_SIZE 10240        

#define DIRECTIONAL_SHADOW_CASCADE_LEVEL 4
#define POINT_SHADOW_MAX_COUNT 4
#define MAX_POINT_LIGHT_COUNT 10
#define MAX_VOLUME_LIGHT_COUNT 10

#define INDIRECT_BUFFER_PRE_PASS 0
#define INDIRECT_BUFFER_DIRECTIONAL_SHADOW 1
#define INDIRECT_BUFFER_POINT_SHADOW 5
#define INDIRECT_BUFFER_FORWARD 9
#define INDIRECT_BUFFER_DEFERRED 10
#define INDIRECT_BUFFER_MAX 11	

struct IndirectCommand {
    uint vertexCount;		
    uint instanceCount;	
    uint firstVertex;		
    uint firstInstance;	
};

struct IndexedIndirectCommand {
    uint    indexCount;
    uint    instanceCount;
    uint    firstIndex;
    uint    vertexOffset;
    uint    firstInstance;
};

struct Vertex {
    vec3 pos;
    vec3 normal;
    vec4 tangent;

    vec3 color;
    vec2 texCoord;

    ivec4 bone_ids;     
    vec4 bone_weights;
};

struct Object {

    mat4 prev_model;
    mat4 model;

    int frame_index;     

    int animationID;
    int materialID;
    int vertexID;
    int indexID;

    BoundingSphere sphere;  
    BoundingBox box;

    vec4 debugData;
};

struct DirectionalLight {
    mat4 view;
    mat4 proj;
    vec3 dir;
    vec4 color_intencity;
    vec4 depth;  

    float fog_scattering;
    float _padding[3];

    Frustum frustum;     
    BoundingSphere sphere;
};

struct PointLight {
    mat4 view[6];
    mat4 proj;
    vec3 pos;
    vec4 color_intencity;
	vec3 near_far_bias;
	vec4 evsm_c1c2;

    bool enable;              
    uint shadowID;           

    float fog_scattering;       
    uint _padding; 

    BoundingSphere sphere;   
};

struct DDGISetting
{
    vec3  grid_start_position;
    vec3  grid_step;
    ivec3 probe_counts;
    vec4 _padding0;

    float depth_sharpness;
    float hysteresis;
    float normal_bias;
    float energy_preservation;

    int   irradiance_texture_width;
    int   irradiance_texture_height;  
    int   depth_texture_width;
    int   depth_texture_height;

    float max_probe_distance;
    int   rays_per_probe;
    float _padding1[2];

    bool enable;
    bool visibility_test;
    bool infinite_bounce;
    bool _padding2;

    BoundingBox box;          
};

struct VolumeLight {
    DDGISetting setting;
};

struct LightSetting {
    uint directional_light_cnt;
    uint pointshadowed_light_cnt;
    uint point_light_cnt;
    uint volume_light_cnt;

    uint global_index_offset;          
    uint _padding[3];                

    uint shadow_light_offsets[POINT_SHADOW_MAX_COUNT];  
};

struct LightIndex
{
    uint index;             
};

struct AnimationInfo {
    mat4 transform[200];
};

struct MaterialInfo {
  
    float roughness;
    float metallic;
    float emission_intencity;
    float alpha_clip;

    vec4 base_color;
    vec4 emission;

    int texture_diffuse;
    int texture_normal;
    int texture_arm;        //AO/Roughness/Metallic
    int texture_specular;

    int ints[8];       
    float floats[8];   

    vec4 colors[8];

    ivec4 texture_slots1_2d; 
    ivec4 texture_slots2_2d;
    ivec4 texture_slots_cube;  
    ivec4 texture_slots_3d;    
};

struct IndirectSetting
{
    uint processSize;   
    uint drawSize;   

    uint frustumCull;               
    uint occlusionCull;          
};

struct BatchInfo {
    uint instanceIndex;			 
    uint batchIndex;				
    uint pipelineID;			   
    uint _padding;
};

struct ClusterInfo
{
    uint vertexID;		      
    uint indexID;
    float lod_error;
    uint _padding;

    BoundingSphere sphere;
};

struct ClusterIndex
{
    uint instanceIndex;               
    uint clusterID;               
};

struct DrawClusterDatas
{
    IndirectSetting setting;
    ClusterIndex indices[MAX_GLOBAL_CLUSTER_SIZE];	
};

struct ClusterGroupInfo
{
    uint clusterID[CLUSTER_GROUP_SIZE]; 

    uint clusterSize;
    float parent_lod_error;  
    uint _padding[2];

    BoundingSphere sphere;
};

struct ClusterGroupIndex
{
    uint instanceIndex;   
    uint clusterGroupID;  
};

struct DrawClusterGroupDatas
{
    IndirectSetting setting;

    ClusterGroupIndex indices[MAX_GLOBAL_CLUSTER_GROUP_SIZE];	    
};

//per frame////////////////////////////////////////////////////////////////////////////

layout(set = 0, binding = 0) buffer global_setting {
    
    int skyboxMaterialID;
    int clusterInspectMode;    
    float totalTickTime;
    int totalTicks;

    float randomSeed[WINDOW_WIDTH][WINDOW_HEIGHT];

} GLOBAL_SETTING;

layout(set = 0, binding = 1) uniform sampler2D NOISE_SAMPLERS[];

//layout(set = 0, binding = 2) uniform accelerationStructureEXT TLAS;

layout(set = 0, binding = 3) uniform camera {

    mat4 view;
    mat4 proj;
    mat4 prev_view;
    mat4 prev_proj;
    mat4 inv_view;
    mat4 inv_proj;

    vec4 jitter;     
    vec4 prev_jitter;

    vec4 pos;        
    vec3 front;
    vec3 up;
    vec3 right;
    vec4 near_far;   
    vec4 fov_aspect;   

    vec4 ambient;      

    Frustum frustum;             
} CAMERA;

layout(set = 0, binding = 4) buffer objects {

    Object slot[MAX_PER_FRAME_RESOURCE_SIZE];

} OBJECTS;

layout(set = 0, binding = 5) buffer light {

    DirectionalLight directionalLights[DIRECTIONAL_SHADOW_CASCADE_LEVEL];
    PointLight pointLights[MAX_POINT_LIGHT_COUNT];
    VolumeLight volumeLights[MAX_VOLUME_LIGHT_COUNT];
    LightSetting lightSetting;

} LIGHT;

layout(set = 0, binding = 6, rg32ui) uniform uimage3D LIGHT_CLUSTER;

layout(set = 0, binding = 7) buffer light_index {

    LightIndex slot[MAX_LIGHTS_PER_CLUSTER * LIGHT_CLUSTER_NUM];

} LIGHT_INDEX;

layout(set = 0, binding = 8) buffer materials { 

    MaterialInfo slot[MAX_PER_FRAME_RESOURCE_SIZE];

} MATERIALS;

layout(set = 0, binding = 9) buffer draw_clusters {
    IndirectCommand command;
    DrawClusterDatas data;

} DRAW_CLUSTERS[];

layout(set = 0, binding = 10) buffer clusters {

   ClusterInfo slot[MAX_GLOBAL_CLUSTER_SIZE];

} CLUSTERS;

layout(set = 0, binding = 11) buffer cluster_groups {

   ClusterGroupInfo slot[MAX_GLOBAL_CLUSTER_GROUP_SIZE];

} CLUSTER_GROUPS;

layout(set = 0, binding = 12) buffer animations {

   AnimationInfo slot[MAX_PER_FRAME_RESOURCE_SIZE * FRAMES_IN_FLIGHT];

} ANIMATIONS;

layout(set = 0, binding = 13) buffer exposure {
  float minLog2Luminance;       //最小亮度，对数
  float inverseLuminanceRange;  //亮度范围，对数倒数
  float luminanceRange;          //亮度范围，对数
  float numPixels;               //总像素数
  float timeCoeff;               //时间加权

  float luminance;                //计算得到的曝光值

  uint histogramBuffer[];        //直方图数组

} EXPOSURE;

layout(set = 0, binding = 14) buffer vertices { 

    Vertex slot[];

} VERTICES[];

layout(set = 0, binding = 15) buffer indices { 

    uint slot[];

} INDICES[];

layout(set = 0, binding = 16) uniform sampler2D TEXTURES_2D[];
layout(set = 0, binding = 17) uniform samplerCube TEXTURES_CUBE[];
layout(set = 0, binding = 18) uniform sampler3D TEXTURES_3D[];

layout(set = 0, binding = 19) uniform sampler2D COLOR_0_SAMPLER;
layout(set = 0, binding = 20) uniform sampler2D COLOR_1_SAMPLER;
layout(set = 0, binding = 21) uniform sampler2D COLOR_DEFERRED_SAMPLER;
layout(set = 0, binding = 22) uniform sampler2D DEPTH_SAMPLER;
layout(set = 0, binding = 23) uniform sampler2D DEPTH_PYRAMID_SAMPLER_MAX;
layout(set = 0, binding = 24) uniform sampler2D DEPTH_PYRAMID_SAMPLER_MIN;
layout(set = 0, binding = 25) uniform sampler2D NORMAL_SAMPLER;
layout(set = 0, binding = 26) uniform sampler2D VELOCITY_SAMPLER;
layout(set = 0, binding = 27) uniform sampler2D EMISSION_SAMPLER;
layout(set = 0, binding = 28) uniform sampler2D AO_SAMPLER;
layout(set = 0, binding = 29, rgba16f) uniform readonly image2D VOLUMETRIC_FOG;
layout(set = 0, binding = 30, rgba16f) uniform readonly image2D STOCHASTIC_SSR;

layout(set = 0, binding = 31) uniform sampler2DArray DIRECTIONAL_SHADOW_SAMPLER;
layout(set = 0, binding = 32) uniform samplerCube POINT_SHADOW_SAMPLERS[];
layout(set = 0, binding = 33) uniform samplerCube POINT_SHADOW_DEPTH_SAMPLERS[];
layout(set = 0, binding = 34) uniform sampler2D VOLUME_LIGHT_IRRADIANCE_SAMPLERS[];
layout(set = 0, binding = 35) uniform sampler2D VOLUME_LIGHT_DEPTH_SAMPLERS[];

layout(set = 0, binding = 36) uniform samplerCube DIFFUSE_IBL_SAMPLER;
layout(set = 0, binding = 37) uniform samplerCube SPECULAR_IBL_SAMPLER;
layout(set = 0, binding = 38) uniform sampler2D BRDF_LUT_SAMPLER;






//顶点////////////////////////////////////////////////////////////////////////////

Vertex barycentricsInterpolation(in Vertex v0, in Vertex v1, in Vertex v2, vec3 barycentrics)
{
  Vertex vert;
  vert.pos      = barycentricsInterpolation(v0.pos, v1.pos, v2.pos, barycentrics);
  vert.normal   = normalize(barycentricsInterpolation(v0.normal, v1.normal, v2.normal, barycentrics));     //切线和法线先插值再变换和先变换再插值的顺序
  vert.tangent  = normalize(barycentricsInterpolation(v0.tangent, v1.tangent, v2.tangent, barycentrics));  //检查了，应该没问题
  vert.color    = barycentricsInterpolation(v0.color, v1.color, v2.color, barycentrics);
  vert.texCoord = barycentricsInterpolation(v0.texCoord, v1.texCoord, v2.texCoord, barycentrics);
  // vert.bone_ids;       //TODO 此处没有考虑骨骼动画
  // vert.bone_weights; 

  return vert;
}

Vertex fetchVertex(in Object object, int triangleID, vec3 barycentrics)
{
  // Indices of the triangle
  uint i0 = INDICES[object.indexID].slot[triangleID * 3];
  uint i1 = INDICES[object.indexID].slot[triangleID * 3 + 1];
  uint i2 = INDICES[object.indexID].slot[triangleID * 3 + 2];
  
  // Vertex of the triangle
  // Vertex v0 = VERTICES[object.indexID].slot[i0];
  // Vertex v1 = VERTICES[object.indexID].slot[i1];
  // Vertex v2 = VERTICES[object.indexID].slot[i2];

  Vertex vert = barycentricsInterpolation(
    VERTICES[object.indexID].slot[i0],
    VERTICES[object.indexID].slot[i1],
    VERTICES[object.indexID].slot[i2],
    barycentrics);

  return vert;
}

Vertex vertexTransform(in Vertex vertex, mat4 transform)
{
  //施密特正交化的tbn矩阵，ogl
  mat3 tbnModel   = mat3(transform);
  vec3 T          = normalize(tbnModel * vertex.tangent.xyz);
  vec3 N          = normalize(tbnModel * vertex.normal.xyz);
  // re-orthogonalize T with respect to N
  //T             = normalize(T - dot(T, N) * N);

  Vertex vert   = vertex;
  vert.pos      = (transform * vec4(vert.pos, 1.0f)).xyz;
  vert.normal   = N;
  vert.tangent  = vec4(T, vertex.tangent.w);  //w分量是手性
  return vert;
}

//texture////////////////////////////////////////////////////////////////////////////

vec4 fetchTex2D(int slot, vec2 coord) {
	return texture(TEXTURES_2D[slot], coord);   
}

vec4 fetchTexCube(int slot, vec3 vector) {
	return texture(TEXTURES_CUBE[slot], vector);   
}

vec4 fetchTex3D(int slot, vec3 vector) {
	return texture(TEXTURES_3D[slot], vector);   
}

//material////////////////////////////////////////////////////////////////////////////

MaterialInfo fetchMaterial(in Object object) {
	return MATERIALS.slot[object.materialID]; 
}

int fetchMaterialInt(in MaterialInfo material, int index) {
	return material.ints[index]; 
}

float fetchMaterialFloat(in MaterialInfo material, int index) {
	return material.floats[index]; 
}

vec4 fetchMaterialColor(in MaterialInfo material, int index) {
	return material.colors[index]; 
}

vec4 fetchMaterialTex2D(in MaterialInfo material, int index, vec2 coord) {
	if(index < 4)   return fetchTex2D(material.texture_slots1_2d[index], coord);
    else            return fetchTex2D(material.texture_slots2_2d[index - 4], coord);
}

vec4 fetchMaterialTexCube(in MaterialInfo material, int index, vec3 vector) {
	return fetchTexCube(material.texture_slots_cube[index], vector);
}

vec4 fetchMaterialTex3D(in MaterialInfo material, int index, vec3 vector) {
	return fetchTex3D(material.texture_slots_3d[index], vector);
}

vec4 fetchBaseColor(in MaterialInfo material){
    return material.base_color;  
}

float fetchRoughness(in MaterialInfo material, vec2 coord){
    if(material.texture_arm > 0)        
    {
        vec3 arm = fetchTex2D(material.texture_arm, coord).rgb;
        arm = pow(arm, vec3(1.0/2.2));          //gamma矫正

        return arm.y;
    }
    else return clamp(material.roughness, 0.00001, 0.99999); 
}

float fetchMetallic(in MaterialInfo material, vec2 coord){
    if(material.texture_arm > 0)        
    {
        vec3 arm = fetchTex2D(material.texture_arm, coord).rgb;
        arm = pow(arm, vec3(1.0/2.2));          //gamma矫正

        return arm.z;
    }
    else return clamp(material.metallic, 0.00001, 0.99999);   
}

float fetchAO(in MaterialInfo material, vec2 coord){
    if(material.texture_arm > 0)        
    {
        vec3 arm = fetchTex2D(material.texture_arm, coord).rgb;
        arm = pow(arm, vec3(1.0/2.2));          //gamma矫正

        return arm.x;
    }
    else return 1.0f;   
}

vec3 fetchEmission(in MaterialInfo material){
    vec3 emission = material.emission.xyz;                                        
    emission *= material.emission_intencity;   

    return emission;
}

vec4 fetchDiffuse(in MaterialInfo material, vec2 coord) {
    if(material.texture_diffuse > 0)    
    {
        vec4 diffuse = fetchTex2D(material.texture_diffuse, coord);
        diffuse = pow(diffuse, vec4(1.0/2.2));          //gamma矫正
        diffuse = fetchBaseColor(material) * diffuse;         

        return diffuse;
    }
    else return fetchBaseColor(material);
}

vec3 fetchNormal(in MaterialInfo material, vec2 coord, vec3 normal, vec4 tangent) {
	if(material.texture_normal > 0)     
    {
        //计算每像素的tbn矩阵可以避免在vert shader输出上的额外两个vec3的插值，其实还会更快！
        float fSign = tangent.w < 0 ? -1 : 1;        
        highp vec3 n = normalize(normal);
        highp vec3 t = normalize(tangent.xyz);       
        highp vec3 b = -fSign * normalize(cross(n, t)); //排列组合试出来的 nm的 为什么
        t = fSign * normalize(cross(n, t));

        highp mat3 TBN = mat3(t, b, n);

        vec3 texNormal = fetchTex2D(material.texture_normal, coord).xyz;
        vec3 outNormal = normalize(texNormal * 2.0 - 1.0);  
        outNormal = normalize(TBN * outNormal);

        return outNormal;
    }

    else return normal;
}

vec4 fetchArm(in MaterialInfo material, vec2 coord) {
	if(material.texture_arm > 0)        return fetchTex2D(material.texture_arm, coord);
    else return vec4(1.0f, 1.0f, 0.0f, 0.0f);
}

vec4 fetchSpecular(in MaterialInfo material, vec2 coord) {
	if(material.texture_specular > 0)        return fetchTex2D(material.texture_specular, coord);
    else return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

//默认纹理////////////////////////////////////////////////////////////////////////////

vec3 fetchSkyLight(vec3 sampleVec) { 
    MaterialInfo material = MATERIALS.slot[GLOBAL_SETTING.skyboxMaterialID]; 

    vec3 sky        = fetchMaterialTexCube(material, 0, sampleVec).rgb;
    float intencity = fetchMaterialFloat(material, 0);

    vec3 skyLight   = pow(sky, vec3(1.0/2.2)) * intencity;
    return skyLight;
}

vec2 fetchNoise2D(ivec2 texPos) {
    return texelFetch(NOISE_SAMPLERS[0], texPos % 512, 0).rg;
}

vec3 fetchNoise3D(ivec2 texPos) {
    return texelFetch(NOISE_SAMPLERS[1], texPos % 512, 0).rgb;
}

float fetchDepth(vec2 coord) {
	return texture(DEPTH_SAMPLER, coord).r;
}

vec3 fetchVelocity(vec2 coord) {
	return texture(VELOCITY_SAMPLER, coord).rgb;
}

vec3 fetchNormal(vec2 coord) {
	return texture(NORMAL_SAMPLER, coord).rgb;
}

float fetchRoughness(vec2 coord) {
	return texture(VELOCITY_SAMPLER, coord).a;  //目前粗糙度放在速度的A通道
}

float fetchMetallic(vec2 coord) {
	return texture(NORMAL_SAMPLER, coord).a;    //目前金属度放在法线的A通道
}

vec4 fetchDiffuse(vec2 coord) {
    return texture(COLOR_DEFERRED_SAMPLER, coord).rgba;    
}

//坐标转换////////////////////////////////////////////////////////////////////////////

//坐标系
//world space:      x正向向前，y正向向上，z正向向右
//view space:       x正向向右，y正向向上，z负向向前
//clip space:       x正向向右，y负向向上，z正向向前
//NDC space;        左上[-1, -1]，右下[1, 1]（透视除法后）
//screen space:     左上[0, 0]，右下[1, 1]

vec2 ndcToScreen(vec2 ndc)
{
    return ndc.xy * 0.5 + 0.5;
}

vec2 screenToNdc(vec2 screen)
{
    return screen.xy * 2.0 - 1.0;
}

vec4 ndcToView(vec4 ndc)
{	
	vec4 view = CAMERA.inv_proj * ndc;      // View space position.
	view /= view.w;                         // Perspective projection.

	return view;
}

vec4 viewToClip(vec4 view)
{
    vec4 clip = CAMERA.proj * view;

    return clip;
}

vec4 viewToNdc(vec4 view)
{
    vec4 ndc = CAMERA.proj * view;
    ndc /= ndc.w;

    return ndc;
}  

vec2 viewToScreen(vec4 view)
{
    return ndcToScreen(viewToNdc(view).xy);
}  

vec4 viewToWorld(vec4 view)
{
    return CAMERA.inv_view * view;
}  

vec4 worldToView(vec4 world)
{
    return CAMERA.view * world;
}  

vec4 worldToCLip(vec4 world)
{
    return viewToClip(worldToView(world));
}

vec4 worldToNdc(vec4 world)
{
    return viewToNdc(worldToView(world));
}  

vec2 worldToScreen(vec4 world)
{
    return viewToScreen(worldToView(world));
}

vec4 ndcToWorld(vec4 ndc)
{	
	return viewToWorld(ndcToView(ndc));
}

vec4 screenToView(vec2 coord, float depth)
{
    vec4 ndc = vec4(screenToNdc(coord), depth, 1.0f);  
	return ndcToView(ndc);
}

vec4 screenToWorld(vec2 coord, float depth)
{
    return viewToWorld(screenToView(coord, depth));
}

vec4 depthToView(vec2 coord)
{
    return screenToView(coord, fetchDepth(coord));
}

vec4 depthToWorld(vec2 coord)
{
    return viewToWorld(depthToView(coord));
}     


//辅助函数////////////////////////////////////////////////////////////////////////////

vec2 fetchScreenPixPos(vec2 coord)
{
	return vec2(coord.x * WINDOW_WIDTH, coord.y * WINDOW_HEIGHT);
}

vec2 fetchScreenPixPos(vec3 ndc)
{
    return fetchScreenPixPos(ndcToScreen(ndc.xy));
}

vec2 fetchHalfScreenPixPos(vec2 coord)
{
	return vec2(coord.x * HALF_WINDOW_WIDTH, coord.y * HALF_WINDOW_HEIGHT);
}

vec2 fetchHalfScreenPixPos(vec3 ndc)
{
    return fetchHalfScreenPixPos(ndcToScreen(ndc.xy));
}

//cluster lighting 辅助函数////////////////////////////////////////////////////////////////////////////

ivec3 fetchLightClusterID(vec3 ndc)
{
    vec2 screenPixPos   = fetchScreenPixPos(ndc);

    float depth         = ndc.z;   
    float linearDepth   = LinearEyeDepth(depth, CAMERA.near_far.x, CAMERA.near_far.y);

    uint clusterZ       = uint(TILE_DEPTH_SIZE * log(linearDepth / CAMERA.near_far.x) / log(CAMERA.near_far.y / CAMERA.near_far.x));  //对数划分
    //uint clusterZ     = uint( (linearDepth - CAMERA.near_far.x) / ((CAMERA.near_far.y - CAMERA.near_far.x) / TILE_DEPTH_SIZE));   //均匀划分
    ivec3 clusterID     = ivec3(uint(screenPixPos.x / TILE_BLOCK_PIX_SIZE), uint(screenPixPos.y / TILE_BLOCK_PIX_SIZE), clusterZ);

    return clusterID;
}

ivec3 fetchLightClusterID(vec2 coord)
{
    float depth         = fetchDepth(coord);   
    vec3 ndc            = vec3(screenToNdc(coord), depth);

    return fetchLightClusterID(ndc);
}

uvec2 fetchLightClusterIndex(ivec3 clusterID)
{
    return imageLoad(LIGHT_CLUSTER, clusterID).xy;
}




#include "common_light.glsl"
#include "common_ddgi.glsl"
#include "lighting.glsl"

#endif

