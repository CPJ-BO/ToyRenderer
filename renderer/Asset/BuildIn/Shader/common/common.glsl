#ifndef COMMON_GLSL
#define COMMON_GLSL


#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_samplerless_texture_functions : enable
#extension GL_ARB_separate_shader_objects : enable

#include "math.glsl"
#include "intersection.glsl"

#define FRAMES_IN_FLIGHT 3							//帧缓冲数目
#define WINDOW_WIDTH 2048                           //32 * 64   16 * 128
#define WINDOW_HEIGHT 1152                          //18 * 64   9 * 128
//#define WINDOW_WIDTH 1920                           
//#define WINDOW_HEIGHT 1080    

#define HALF_WINDOW_WIDTH (WINDOW_WIDTH / 2)
#define HALF_WINDOW_HEIGHT (WINDOW_HEIGHT / 2)

#define MAX_BINDLESS_RESOURCE_SIZE 10240	        //bindless 单个binding的最大描述符数目
#define MAX_PER_FRAME_RESOURCE_SIZE 10240			//全局的缓冲大小（个数）,包括动画，材质等
#define MAX_PER_FRAME_OBJECT_SIZE 10240			    //全局最大支持的物体数目

#define DIRECTIONAL_SHADOW_SIZE 4096				//方向光源尺寸
#define DIRECTIONAL_SHADOW_CASCADE_LEVEL 4			//CSM级数

#define POINT_SHADOW_SIZE 512						//点光源尺寸，分辨率对性能影响也比较大
#define MAX_POINT_SHADOW_COUNT 4					//阴影点光源最大数目
#define MAX_POINT_LIGHT_COUNT 10240					//点光源最大数目
#define MAX_VOLUME_LIGHT_COUNT 100                   //体积光源最大数目

#define CLUSTER_TRIANGLE_SIZE 128                   //每个cluster内的三角形数目，固定尺寸
#define CLUSTER_GROUP_SIZE 32                       //每个cluster ghroup内的最大cluster数目
#define MAX_PER_FRAME_CLUSTER_SIZE 1024000          //全局最大支持的cluster数目
#define MAX_PER_FRAME_CLUSTER_GROUP_SIZE 102400     //全局最大支持的cluster group数目
#define MAX_PER_PASS_PIPELINE_STATE_COUNT 1024      //每个mesh pass支持的最大的不同管线状态数目
#define MAX_SUPPORTED_MESH_PASS_COUNT 256           //全局支持的最大mesh pass数目 

#define MAX_LIGHTS_PER_CLUSTER 8                    //每个cluster最多支持存储的光源数目
#define LIGHT_CLUSTER_GRID_SIZE 64                  //cluster based lighting裁剪时使用的tile像素尺寸
#define LIGHT_CLUSTER_DEPTH 128                     //cluster based lighting裁剪时Z轴的划分数量    
#define LIGHT_CLUSTER_WIDTH (WINDOW_WIDTH / LIGHT_CLUSTER_GRID_SIZE)    // X轴
#define LIGHT_CLUSTER_HEIGHT (WINDOW_HEIGHT / LIGHT_CLUSTER_GRID_SIZE)  // Y轴
#define LIGHT_CLUSTER_NUM   (LIGHT_CLUSTER_WIDTH * LIGHT_CLUSTER_HEIGHT * LIGHT_CLUSTER_DEPTH)

///////////////////////////////////////////////////////////////////////////////

#define PER_FRAME_BINDING_GLOBAL_SETTING                0
#define PER_FRAME_BINDING_CAMERA                        1
#define PER_FRAME_BINDING_OBJECT                        2
#define PER_FRAME_BINDING_MATERIAL                      3
#define PER_FRAME_BINDING_LIGHT                         4
#define PER_FRAME_BINDING_LIGHT_CLUSTER_GRID            5
#define PER_FRAME_BINDING_LIGHT_CLUSTER_INDEX           6
#define PER_FRAME_BINDING_DIRECTIONAL_SHADOW            7
#define PER_FRAME_BINDING_POINT_SHADOW                  8
#define PER_FRAME_BINDING_MESH_CLUSTER                  9
#define PER_FRAME_BINDING_MESH_CLUSTER_GROUP            10  
#define PER_FRAME_BINDING_MESH_CLUSTER_DRAW_INFO        11
#define PER_FRAME_BINDING_DEPTH                         12
#define PER_FRAME_BINDING_DEPTH_PYRAMID                 13
#define PER_FRAME_BINDING_VELOCITY                      14
#define PER_FRAME_BINDING_VERTEX                        15
#define PER_FRAME_BINDING_BINDLESS_POSITION             16
#define PER_FRAME_BINDING_BINDLESS_NORMAL               17
#define PER_FRAME_BINDING_BINDLESS_TANGENT              18
#define PER_FRAME_BINDING_BINDLESS_TEXCOORD             19
#define PER_FRAME_BINDING_BINDLESS_COLOR                20
#define PER_FRAME_BINDING_BINDLESS_BONE_INDEX           21
#define PER_FRAME_BINDING_BINDLESS_BONE_WEIGHT          22
#define PER_FRAME_BINDING_BINDLESS_ANIMATION            23
#define PER_FRAME_BINDING_BINDLESS_INDEX                24
#define PER_FRAME_BINDING_BINDLESS_SAMPLER              25
#define PER_FRAME_BINDING_BINDLESS_TEXTURE_1D           26
#define PER_FRAME_BINDING_BINDLESS_TEXTURE_1D_ARRAY     27
#define PER_FRAME_BINDING_BINDLESS_TEXTURE_2D           28
#define PER_FRAME_BINDING_BINDLESS_TEXTURE_2D_ARRAY     29
#define PER_FRAME_BINDING_BINDLESS_TEXTURE_CUBE         30
#define PER_FRAME_BINDING_BINDLESS_TEXTURE_3D           31



struct Object 
{
    mat4 prevModel;
    mat4 model; 

    uint animationID;           //动画buffer索引
    uint materialID;            //材质buffer索引
    uint vertexID;
    uint indexID;           

    BoundingSphere sphere;  
    BoundingBox box;

    vec4 debugData;
};

struct VertexStream
{
    uint positionID;
    uint normalID;
    uint tangentID;
    uint texCoordID;
    uint colorID;
    uint boneIndexID;              
    uint boneWeightID;
    uint _padding;
};

struct Material 
{
    float roughness;
    float metallic;
    float alphaClip;

    vec4 baseColor;
    vec4 emission;

    uint textureDiffuse;
    uint textureNormal;
    uint textureArm;        //AO/Roughness/Metallic
    uint textureSpecular;

    int ints[8];       
    float floats[8];   

    vec4 colors[8];

    uint textureSlots2D[8]; 
    uint textureSlotsCube[4];
    uint textureSlots3D[4];  
};

struct LightClusterIndex
{
    uint lightID;             
};

struct DirectionalLight
{
    mat4 view;
    mat4 proj;
    vec3 dir;
    float depth;              

    vec3 color;
    float intencity;            
    
    float fogScattering;       
    float _padding[3];

    Frustum frustum;            
    BoundingSphere sphere;
};

struct PointLight
{
    mat4 view[6];
    mat4 proj;
    vec3 pos;
    vec3 color;
    float intencity;
    float near;
    float far;                             
    float bias;
    float _padding1;
     
    float c1;                          
    float c2;
    bool enable;
    uint shadowID;                    

    float fogScattering;                 
    float _padding2[3];

    BoundingSphere sphere;                
};

struct DDGISetting
{
    vec3  gridStartPosition;
    vec3  gridStep;
    ivec3 probecounts;
    float _padding2;

    float depthSharpness;
    float hysteresis;
    float normalBias;
    float energyPreservation;

    uint irradianceTextureWidth;
    uint irradianceTextureHeight;
    uint depthTextureWidth;
    uint depthTextureWeight;

    float maxProbeDistance;
    int   raysPerProbe;
    float _padding3[2];

    bool enable; 
    bool visibilityTest; 
    bool infiniteBounce;
    bool _padding4;

    BoundingBox boundingBox;   
};

struct VolumeLight
{
    DDGISetting setting;
};

struct LightSetting 
{
    uint directionalLightCnt;
    uint pointshadowedLightCnt;
    uint pointLightCnt;
    uint volumeLightCnt;

    uint globalIndexOffset;                          
    uint _padding[3];

    uint pointLightIDs[MAX_POINT_LIGHT_COUNT];
    uint pointShadowLightIDs[MAX_POINT_SHADOW_COUNT];  
};

struct MeshCluster
{
    uint vertexID;		      
    uint indexID;
    uint indexOffset;
    float lodError;

    BoundingSphere sphere;
};

struct MeshClusterGroup
{
    uint clusterID[CLUSTER_GROUP_SIZE]; 

    uint clusterSize;
    float parentLodError;  
    uint mipLevel;
    float _padding;

    BoundingSphere sphere;
};

struct MeshClusterDrawInfo
{
    uint objectID;      // 每个cluster对应的物体的实例索引
    uint clusterID;     // 对cluster的索引
};

layout(set = 0, binding = PER_FRAME_BINDING_GLOBAL_SETTING) buffer global_setting {
    
    uint skyboxMaterialID;
    uint clusterInspectMode;    
    uint totalTicks;
    float totalTickTime;
    float minFrameTime;

} GLOBAL_SETTING;

layout(set = 0, binding = PER_FRAME_BINDING_CAMERA) buffer camera {

    mat4 view;
    mat4 proj;
    mat4 prevView;
    mat4 prevProj;
    mat4 invView;
    mat4 invProj;

    vec4 pos;  

    vec3 front;
    vec3 up;
    vec3 right;
    float _padding;

    float near;   
    float far;
    float fov;
    float aspect;       

    Frustum frustum;
            
} CAMERA;

layout(set = 0, binding = PER_FRAME_BINDING_OBJECT) buffer objects {

    Object slot[MAX_PER_FRAME_RESOURCE_SIZE];

} OBJECTS;

layout(set = 0, binding = PER_FRAME_BINDING_MATERIAL) buffer materials { 

    Material slot[MAX_PER_FRAME_RESOURCE_SIZE];

} MATERIALS;

layout(set = 0, binding = PER_FRAME_BINDING_LIGHT) buffer lights {

    DirectionalLight directionalLights[DIRECTIONAL_SHADOW_CASCADE_LEVEL];
    PointLight pointLights[MAX_POINT_LIGHT_COUNT];
    VolumeLight volumeLights[MAX_VOLUME_LIGHT_COUNT];
    LightSetting lightSetting;

} LIGHTS;

layout(set = 0, binding = PER_FRAME_BINDING_LIGHT_CLUSTER_GRID, rg32ui) uniform uimage3D LIGHT_CLUSTER_GRID;

layout(set = 0, binding = PER_FRAME_BINDING_LIGHT_CLUSTER_INDEX) buffer light_cluster_index {

    LightClusterIndex slot[MAX_LIGHTS_PER_CLUSTER * LIGHT_CLUSTER_NUM];

} LIGHT_CLUSTER_INDEX;

layout(set = 0, binding = PER_FRAME_BINDING_DIRECTIONAL_SHADOW) uniform texture2D DIRECTIONAL_SHADOW[];

layout(set = 0, binding = PER_FRAME_BINDING_POINT_SHADOW) uniform textureCube POINT_SHADOW[];

layout(set = 0, binding = PER_FRAME_BINDING_MESH_CLUSTER) buffer mesh_clusters { 

    MeshCluster slot[MAX_PER_FRAME_CLUSTER_SIZE];

} MESH_CLUSTERS;

layout(set = 0, binding = PER_FRAME_BINDING_MESH_CLUSTER_GROUP) buffer mesh_cluster_groups { 

    MeshClusterGroup slot[MAX_PER_FRAME_CLUSTER_SIZE];

} MESH_CLUSTER_GROUPS;

layout(set = 0, binding = PER_FRAME_BINDING_MESH_CLUSTER_DRAW_INFO) buffer mesh_cluster_draw_infos { 

    MeshClusterDrawInfo slot[MAX_PER_FRAME_CLUSTER_SIZE * MAX_SUPPORTED_MESH_PASS_COUNT];

} MESH_CLUSTER_DRAW_INFOS;

layout(set = 0, binding = PER_FRAME_BINDING_DEPTH) uniform texture2D DEPTH;

layout(set = 0, binding = PER_FRAME_BINDING_DEPTH_PYRAMID) uniform texture2D DEPTH_PYRAMID[2];  // MIN, MAX

layout(set = 0, binding = PER_FRAME_BINDING_VELOCITY) uniform texture2D VELOCITY;

layout(set = 0, binding = PER_FRAME_BINDING_VERTEX) buffer vertices { 

    VertexStream slot[MAX_PER_FRAME_RESOURCE_SIZE];

} VERTICES;

layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_POSITION) buffer positions { 

    float position[];

} POSITIONS[MAX_BINDLESS_RESOURCE_SIZE];

layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_NORMAL) buffer normals { 

    float normal[];

} NORMALS[MAX_BINDLESS_RESOURCE_SIZE];


layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_TANGENT) buffer tangents { 

    float tangent[];

} TANGENTS[MAX_BINDLESS_RESOURCE_SIZE];


layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_TEXCOORD) buffer texCoords { 

    float texCoord[];

} TEXCOORDS[MAX_BINDLESS_RESOURCE_SIZE];


layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_COLOR) buffer colors { 

    float color[];

} COLORS[MAX_BINDLESS_RESOURCE_SIZE];

layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_BONE_INDEX) buffer boneIndexs { 

    int boneIndex[];

} BONEINDEXS[MAX_BINDLESS_RESOURCE_SIZE];

layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_BONE_WEIGHT) buffer boneWeights { 

    float boneWeight[];

} BONEWEIGHTS[MAX_BINDLESS_RESOURCE_SIZE];

layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_ANIMATION) buffer animations { 

    mat4 matrix[];

} ANIMATIONS[MAX_BINDLESS_RESOURCE_SIZE];

layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_INDEX) buffer indices { 

    uint index[];

} INDICES[MAX_BINDLESS_RESOURCE_SIZE];

layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_SAMPLER) uniform sampler SAMPLER[];
layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_TEXTURE_1D) uniform texture1D TEXTURES_1D[];
layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_TEXTURE_1D_ARRAY) uniform texture1DArray TEXTURES_1D_ARRAY[];
layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_TEXTURE_2D) uniform texture2D TEXTURES_2D[];
layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_TEXTURE_2D_ARRAY) uniform texture2DArray TEXTURES_2D_ARRAY[];
layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_TEXTURE_CUBE) uniform textureCube TEXTURES_CUBE[];
layout(set = 0, binding = PER_FRAME_BINDING_BINDLESS_TEXTURE_3D) uniform texture3D TEXTURES_3D[];

//顶点数据////////////////////////////////////////////////////////////////////////////

mat4 FetchModel(uint objectID)
{
    return OBJECTS.slot[objectID].model;
}

uint FetchIndex(uint objectID, uint offset)
{
    uint indexID = OBJECTS.slot[objectID].indexID;
    uint index = INDICES[indexID].index[offset];

    return index;
}

vec4 FetchPos(uint objectID, uint index)
{
    uint vertexID = OBJECTS.slot[objectID].vertexID;
    if(vertexID == 0) return vec4(0.0f);

    uint positionID = VERTICES.slot[vertexID].positionID;
    if(positionID == 0) return vec4(0.0f);

    return vec4(POSITIONS[positionID].position[3 * index], 
                POSITIONS[positionID].position[3 * index + 1], 
                POSITIONS[positionID].position[3 * index + 2],
                1.0f);
}

vec3 FetchNormal(uint objectID, uint index)
{
    uint vertexID = OBJECTS.slot[objectID].vertexID;
    if(vertexID == 0) return vec3(0.0f, 0.0f, 1.0f);

    uint normalID = VERTICES.slot[vertexID].normalID;
    if(normalID == 0) return vec3(0.0f, 0.0f, 1.0f);

    return vec3(NORMALS[normalID].normal[3 * index], 
                NORMALS[normalID].normal[3 * index + 1], 
                NORMALS[normalID].normal[3 * index + 2]);   
}

vec4 FetchTangent(uint objectID, uint index)
{
    uint vertexID = OBJECTS.slot[objectID].vertexID;
    if(vertexID == 0) return vec4(0.0f);

    uint tangentID = VERTICES.slot[vertexID].tangentID;
    if(tangentID == 0) return vec4(0.0f);

    return vec4(TANGENTS[tangentID].tangent[4 * index], 
                TANGENTS[tangentID].tangent[4 * index + 1], 
                TANGENTS[tangentID].tangent[4 * index + 2],
                TANGENTS[tangentID].tangent[4 * index + 3]);   
}

vec3 FetchWorldNormal(vec3 normal, mat4 model)
{
    mat3 tbnModel = mat3(model);
    return normalize(tbnModel * normal);
}

vec4 FetchWorldTangent(vec4 tangent, mat4 model)
{
    mat3 tbnModel = mat3(model);
    return vec4(normalize(tbnModel * tangent.xyz), tangent.w);
}

vec2 FetchTexCoord(uint objectID, uint index)
{
    uint vertexID = OBJECTS.slot[objectID].vertexID;
    if(vertexID == 0) return vec2(0.0f);

    uint texCoordID = VERTICES.slot[vertexID].texCoordID;
    if(texCoordID == 0) return vec2(0.0f);

    return vec2(TEXCOORDS[texCoordID].texCoord[2 * index], 
                TEXCOORDS[texCoordID].texCoord[2 * index + 1]);
}

vec3 FetchColor(uint objectID, uint index)
{
    uint vertexID = OBJECTS.slot[objectID].vertexID;
    if(vertexID == 0) return vec3(0.0f);

    uint colorID = VERTICES.slot[vertexID].colorID;
    if(colorID == 0) return vec3(0.0f);

    return vec3(COLORS[colorID].color[3 * index], 
                COLORS[colorID].color[3 * index + 1],
                COLORS[colorID].color[3 * index + 2]);
}

uvec4 FetchBoneIndex(uint objectID, uint index)
{
    uint vertexID = OBJECTS.slot[objectID].vertexID;
    if(vertexID == 0) return uvec4(0);

    uint boneIndexID = VERTICES.slot[vertexID].boneIndexID;
    if(boneIndexID == 0) return uvec4(0);

    return uvec4(BONEINDEXS[boneIndexID].boneIndex[4 * index], 
                 BONEINDEXS[boneIndexID].boneIndex[4 * index + 1], 
                 BONEINDEXS[boneIndexID].boneIndex[4 * index + 2],
                 BONEINDEXS[boneIndexID].boneIndex[4 * index + 3]);   
}

vec4 FetchBoneWeight(uint objectID, uint index)
{
    uint vertexID = OBJECTS.slot[objectID].vertexID;
    if(vertexID == 0) return vec4(0);

    uint boneWeightID = VERTICES.slot[vertexID].boneWeightID;
    if(boneWeightID == 0) return vec4(0);

    return vec4(BONEWEIGHTS[boneWeightID].boneWeight[4 * index], 
                BONEWEIGHTS[boneWeightID].boneWeight[4 * index + 1], 
                BONEWEIGHTS[boneWeightID].boneWeight[4 * index + 2],
                BONEWEIGHTS[boneWeightID].boneWeight[4 * index + 3]);   
}

//texture////////////////////////////////////////////////////////////////////////////

vec4 FetchTex2D(uint slot, vec2 coord) {
	return texture(sampler2D(TEXTURES_2D[slot], SAMPLER[1]), coord);   
}

vec4 FetchTexCube(uint slot, vec3 vector) {
	return texture(samplerCube(TEXTURES_CUBE[slot], SAMPLER[1]), vector);   
}

vec4 FetchTex3D(uint slot, vec3 vector) {
	return texture(sampler3D(TEXTURES_3D[slot], SAMPLER[1]), vector);   
}

//material////////////////////////////////////////////////////////////////////////////

Material FetchMaterial(uint objectID) {
	return MATERIALS.slot[OBJECTS.slot[objectID].materialID]; 
}

int FetchMaterialInt(in Material material, int index) {
	return material.ints[index]; 
}

float FetchMaterialFloat(in Material material, int index) {
	return material.floats[index]; 
}

vec4 FetchMaterialColor(in Material material, int index) {
	return material.colors[index]; 
}

vec4 FetchMaterialTex2D(in Material material, int index, vec2 coord) {
	return FetchTex2D(material.textureSlots2D[index], coord);
}

vec4 FetchMaterialTexCube(in Material material, int index, vec3 vector) {
	return FetchTexCube(material.textureSlotsCube[index], vector);
}

vec4 FetchMaterialTex3D(in Material material, int index, vec3 vector) {
	return FetchTex3D(material.textureSlots3D[index], vector);
}

vec4 FetchBaseColor(in Material material){
    return material.baseColor;  
}

float FetchRoughness(in Material material, vec2 coord){
    if(material.textureArm > 0)        
    {
        vec3 arm = FetchTex2D(material.textureArm, coord).rgb;
        arm = pow(arm, vec3(1.0/2.2));          //gamma矫正

        return arm.y;
    }
    else return clamp(material.roughness, 0.00001, 0.99999); 
}

float FetchMetallic(in Material material, vec2 coord){
    if(material.textureArm > 0)        
    {
        vec3 arm = FetchTex2D(material.textureArm, coord).rgb;
        arm = pow(arm, vec3(1.0/2.2));          //gamma矫正

        return arm.z;
    }
    else return clamp(material.metallic, 0.00001, 0.99999);   
}

float FetchAO(in Material material, vec2 coord){
    if(material.textureArm > 0)        
    {
        vec3 arm = FetchTex2D(material.textureArm, coord).rgb;
        arm = pow(arm, vec3(1.0/2.2));          //gamma矫正

        return arm.x;
    }
    else return 1.0f;   
}

vec3 FetchEmission(in Material material){
    return material.emission.xyz * material.emission.w; 
}

vec4 FetchDiffuse(in Material material, vec2 coord) {
    if(material.textureDiffuse > 0)    
    {
        vec4 diffuse = FetchTex2D(material.textureDiffuse, coord);
        diffuse = pow(diffuse, vec4(1.0/2.2));          //gamma矫正
        diffuse = FetchBaseColor(material) * diffuse;         

        return diffuse;
    }
    else return FetchBaseColor(material);
}

vec3 FetchNormal(in Material material, vec2 coord, vec3 normal, vec4 tangent) {
	if(material.textureNormal > 0)     
    {
        //计算每像素的tbn矩阵可以避免在vert shader输出上的额外两个vec3的插值，其实还会更快！
        float fSign = tangent.w < 0 ? -1 : 1;        
        highp vec3 n = normalize(normal);
        highp vec3 t = normalize(tangent.xyz);       
        highp vec3 b = -fSign * normalize(cross(n, t)); //排列组合试出来的 nm的 为什么
        t = fSign * normalize(cross(n, t));

        highp mat3 TBN = mat3(t, b, n);

        vec3 texNormal = FetchTex2D(material.textureNormal, coord).xyz;
        vec3 outNormal = normalize(texNormal * 2.0 - 1.0);  
        outNormal = normalize(TBN * outNormal);

        return outNormal;
    }

    else return normal;
}

vec4 FetchArm(in Material material, vec2 coord) {
	if(material.textureArm > 0)        return FetchTex2D(material.textureArm, coord);
    else return vec4(1.0f, 1.0f, 0.0f, 0.0f);
}

vec4 FetchSpecular(in Material material, vec2 coord) {
	if(material.textureSpecular > 0)        return FetchTex2D(material.textureSpecular, coord);
    else return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

//默认纹理////////////////////////////////////////////////////////////////////////////

// vec3 FetchSkyLight(vec3 sampleVec) { 
//     Material material = MATERIALS.slot[GLOBAL_SETTING.skyboxMaterialID]; 

//     vec3 sky        = FetchMaterialTexCube(material, 0, sampleVec).rgb;
//     float intencity = FetchMaterialFloat(material, 0);

//     vec3 skyLight   = pow(sky, vec3(1.0/2.2)) * intencity;
//     return skyLight;
// }

// vec2 FetchNoise2D(ivec2 texPos) {
//     return texelFetch(NOISE_SAMPLERS[0], texPos % 512, 0).rg;
// }

// vec3 FetchNoise3D(ivec2 texPos) {
//     return texelFetch(NOISE_SAMPLERS[1], texPos % 512, 0).rgb;
// }

float FetchDepth(vec2 coord) {
	return texture(sampler2D(DEPTH, SAMPLER[1]), coord).r;
}

// vec3 FetchVelocity(vec2 coord) {
// 	return texture(VELOCITY_SAMPLER, coord).rgb;
// }

// vec3 FetchNormal(vec2 coord) {
// 	return texture(NORMAL_SAMPLER, coord).rgb;
// }

// float FetchRoughness(vec2 coord) {
// 	return texture(VELOCITY_SAMPLER, coord).a;  //目前粗糙度放在速度的A通道
// }

// float FetchMetallic(vec2 coord) {
// 	return texture(NORMAL_SAMPLER, coord).a;    //目前金属度放在法线的A通道
// }

// vec4 FetchDiffuse(vec2 coord) {
//     return texture(COLOR_DEFERRED_SAMPLER, coord).rgba;    
// }

#include "coordinate.glsl"
#include "screen.glsl"
#include "light.glsl"
#include "brdf.glsl"


#endif