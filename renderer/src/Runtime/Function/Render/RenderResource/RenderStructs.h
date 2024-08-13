#pragma once

#include "Core/Math/BoundingBox.h"
#include "Core/Math/Math.h"
#include "Function/Global/Definations.h"

#include <array>
#include <cstdint>

// 声明各类在GPU和CPU间传递信息的数据结构
// 注意buffer的字节对齐问题

typedef struct RenderGlobalSetting
{
    int skyboxMaterialID;       //天空盒的材质ID
    int clusterInspectMode;     //cluster渲染使用的模式

    float totalTickTime;        //运行时间
    int totalTicks;             //运行帧数
} RenderGlobalSetting;

typedef struct CameraInfo 
{
    Mat4 view;
    Mat4 proj;
    Mat4 prevView;
    Mat4 prevProj;
    Mat4 invView;
    Mat4 invProj;

    Vec3 pos;           //世界坐标
    float _padding0;

    Vec3 front;
    float _padding1;
    Vec3 up;
    float _padding2;
    Vec3 right;
    float _padding3;

    float near;
    float far;
    float fov;
    float aspect;

    Frustum frustum;    //视锥，用于计算剔除

} CameraInfo;

typedef struct ObjectInfo
{
    Mat4 prevModel;                 //前一帧的model矩阵
    Mat4 model;

    uint32_t animationID;           //动画索引
    uint32_t materialID;            //材质索引
    uint32_t positionID;
    uint32_t normalID;
    uint32_t tangentID;
    uint32_t texCoordID;
    uint32_t colorID;
    uint32_t boneIndexID;              
    uint32_t boneWeightID;
    uint32_t indexID;             
    uint32_t _padding[2];  

    BoundingSphere sphere;          //包围球，本地空间
    BoundingBox box;                //包围盒，本地空间

    Vec4 debugData;                 //仅debug使用

} ObjectInfo;

typedef struct MaterialInfo 
{
    //固定槽位///////////////////////////////
    float roughness;
    float metallic;
    float alphaClip;
    float _padding;

    Vec4 diffuse;
    Vec4 emission;

    uint32_t textureDiffuse;
    uint32_t textureNormal;
    uint32_t textureArm;        //AO/Roughness/Metallic
    uint32_t textureSpecular;

    //预留的通用槽位///////////////////////////////
    std::array<int32_t, 8> ints;
    std::array<float, 8> floats;
    std::array<Vec4, 8> colors;

    std::array<uint32_t, 8> texture2D;      //额外的纹理使用的下标，一共支持每材质8个2d纹理，
    std::array<uint32_t, 4> textureCube;    //4个cube纹理
    std::array<uint32_t, 4> texture3D;      //4个3d纹理

} MaterialInfo;

typedef struct DirectionalLightInfo 
{
    Mat4 view;
    Mat4 proj;
    Vec3 dir;
    float depth;                //平行光源级联的划分深度，对齐Vec4

    Vec3 color;
    float intencity;            //前三维为颜色，最后一维强度
    
    float fogScattering;        //体积雾散射系数
    float _padding[3];

    Frustum frustum;            //视锥，用于计算剔除      
    BoundingSphere sphere;

} DirectionalLightInfo;

typedef struct PointLightInfo
{
    Mat4 view[6];
    Mat4 proj;
    Vec3 pos;
    float _padding0;
    Vec3 color;
    float intencity;
    float near;
    float far;                             
    float bias;
    float _padding1;
     
    float c1;                               //EVSM计算时使用的指数系数
    float c2;
    union { uint32_t _p0; bool enable; };   //是否启用
    uint32_t  shadowID;                     //索引点光源阴影的槽位，小于POINT_SHADOW_MAX_COUNT有效，每帧更新

    float fogScattering;                    //体积雾散射系数
    float _padding2[3];

    BoundingSphere sphere;                  //世界空间包围球，用于计算剔除

} PointLightInfo;

typedef struct DDGISetting
{
    Vec3  gridStartPosition;
    float _padding0;
    Vec3  gridStep;
    float _padding1;
    IVec3 probecounts;
    float _padding2;

    float depthSharpness;
    float hysteresis;
    float normalBias;
    float energyPreservation;

    uint32_t irradianceTextureWidth;
    uint32_t irradianceTextureHeight;
    uint32_t depthTextureWidth;
    uint32_t depthTextureWeight;

    float maxProbeDistance;
    int   raysPerProbe;
    float _padding3[2];

    union { uint32_t _p0; bool enable; };   //cpp是一字节，glsl是四字节，手动对齐
    union { uint32_t _p1; bool visibility_test; };
    union { uint32_t _p2; bool infinite_bounce; };
    union { uint32_t _p3; bool _padding4; };

    BoundingBox boundingBox;    //包围盒，世界空间

} DDGISetting;

typedef struct VolumeLightInfo
{
    DDGISetting setting;

} VolumeLightInfo;

typedef struct LightSetting 
{
    uint32_t directionalLightCnt;
    uint32_t pointshadowedLightCnt;
    uint32_t pointLightCnt;
    uint32_t volumeLightCnt;

    uint32_t globalIndexOffset;                           //创建cluster light时使用，标记全局index缓冲的已分配偏移
    uint32_t _padding[3];

    uint32_t shadowLightOffsets[MAX_POINT_SHADOW_COUNT];  //标记阴影点光源的lightID，方便剔除时索引
} LightSetting;

typedef struct LightIndex
{
    uint32_t index;             //光源索引信息，索引LightInfo内的实际光源信息  

} LightIndex;



typedef struct AnimatedTransformInfo 
{
    Mat4 transform[200];

} AnimatedTransformInfo;

typedef struct ExpolureSetting 
{
    float minLog2Luminance;         //最小亮度，对数
    float inverseLuminanceRange;    //亮度范围，对数倒数
    float luminanceRange;           //亮度范围，对数
    float numPixels;                //总像素数
    float timeCoeff;                //时间加权
    float _padding0[3];

} ExpolureSetting;

typedef struct ExpolureInfo 
{
    ExpolureSetting expolureSetting;

    float luminance;                //计算得到的曝光值
    float _padding0[3];
    uint32_t histogram[256];        //直方图数组

} ExpolureInfo;


//Indirect Batch /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct IndirectSetting
{
    uint32_t processSize = 0;               //本轮需要处理的全部batch/cluster/cluster group数目
    uint32_t drawSize = 0;                  //通过culling实际需要绘制的数目，由GPU端计算写入

    uint32_t frustumCull = 0;               //视锥剔除数目
    uint32_t occlusionCull = 0;             //遮蔽剔除数目
} IndirectSetting;

//在单个pass内：
//1. 同mesh，同material，不同instance：允许不同animation，可以使用同一个pipeline和同一个indirectBuffer，	//！！且需要重新分配instanceIndex到邻近！！TODO	
//2. 不同material：未必使用同一个pipeline，需要区分
//3. 不同mesh：不使用同一个indirectBuffer，需要区分
//4. 使用同一个pipeline的可能来自多个batch，可以记录下来，draw的时候少一些绑定
typedef struct BatchInfo
{
    uint32_t instanceIndex = 0;			    // 物体的实例索引
    uint32_t batchIndex = 0;				// 物体的batch索引，实际上也就是inderect draw buffer的offset
    uint32_t pipelineID = 0;			    // 渲染pipeline索引，由材质决定

    uint32_t _padding;
} BatchInfo;

//Indirect Virtual Mesh /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// typedef struct ClusterGroupInfo
// {
//     uint32_t clusterID[CLUSTER_GROUP_SIZE]; //到cluster的索引，绝对偏移

//     uint32_t clusterSize;
//     float parentLodError;
//     uint32_t _padding[2];

//     BoundingSphere sphere;                  //包围球

// } ClusterGroupInfo;

// typedef struct ClusterGroupIndex
// {
//     uint32_t instanceIndex;                 //每个cluster对应的物体的实例索引
//     uint32_t clusterGroupID;                //对ClusterInfoBuffer的索引
//     float _padding[2];

// } ClusterGroupIndex;

// typedef struct DrawClusterGroupDatas
// {
//     IndirectSetting setting;

//     ClusterGroupIndex indices[MAX_GLOBAL_CLUSTER_GROUP_SIZE];	    

// } DrawClusterGroupDatas;

//Indirect Cluster /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// typedef struct clusterInfo
// {
//     uint32_t vertexID = 0;		            //到实际的顶点buffer索引
//     uint32_t indexID = 0;
//     //uint32_t indexCount = CLUSTER_TRIANGLE_SIZE * 3;  
//     float lodError = 0.0f;
//     uint32_t _padding;

//     BoundingSphere sphere;                  //包围球，用于剔除

// } ClusterInfo;

// typedef struct ClusterIndex
// {
//     uint32_t instanceIndex;                 //每个cluster对应的物体的实例索引
//     uint32_t clusterID;                     //对ClusterInfoBuffer的索引

// } ClusterIndex;

// typedef struct DrawClusterDatas
// {
//     IndirectSetting setting;

//     ClusterIndex indices[MAX_GLOBAL_CLUSTER_SIZE];	    //需要固定每个cluster的索引数目（三角形数目），然后使用gl_instanceIndex = clusterID 来索引
                                                      
// } DrawClusterDatas;

// typedef struct DrawClusterBuffer			            //用于收集一帧需要渲染的全部cluster数据，只要使用cluster都往这里填，每个绘制pass一个，三帧
// {
//     RHIIndirectCommand command = {};			        //间接绘制指令，限定了全延迟，仅一个drawcall，不是VkDrawIndirectCommand（索引由gl_VertexID去查）
//     //uint32_t    vertexCount;		= CLUSTER_TRIANGLE_SIZE * 3
//     //uint32_t    instanceCount;	= cluster_num
//     //uint32_t    firstVertex;		= 0
//     //uint32_t    firstInstance;	= 0

//     DrawClusterDatas datas;                             //剔除之后剩余的实际绘制使用的所有cluster
                                                        
// } DrawClusterBuffer;



