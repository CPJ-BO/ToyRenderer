#pragma once

#define FRAMES_IN_FLIGHT 3							//帧缓冲数目
#define WINDOW_WIDTH 2048                           //32 * 64   16 * 128
#define WINDOW_HEIGHT 1152                          //18 * 64   9 * 128
//#define WINDOW_WIDTH 1920                           
//#define WINDOW_HEIGHT 1080    

#define HALF_WINDOW_WIDTH (WINDOW_WIDTH / 2)
#define HALF_WINDOW_HEIGHT (WINDOW_HEIGHT / 2)

#define MAX_BINDLESS_RESOURCE_SIZE 10240	        //bindless 单个binding的最大描述符数目
#define MAX_PER_FRAME_RESOURCE_SIZE 10240			//全局的缓冲大小（个数）,包括动画，材质等

#define DIRECTIONAL_SHADOW_SIZE 4096				//方向光源尺寸
#define DIRECTIONAL_SHADOW_CASCADE_LEVEL 4			//CSM级数

#define POINT_SHADOW_SIZE 512						//点光源尺寸，分辨率对性能影响也比较大
#define MAX_POINT_SHADOW_COUNT 4					//阴影点光源最大数目
#define MAX_POINT_LIGHT_COUNT 10					//点光源最大数目
#define MAX_VOLUME_LIGHT_COUNT 10                   //体积光源最大数目


// #define DDGI_IRRADIANCE_PROBE_SIZE 8                //DDGI使用的辐照度贴图，单个probe的纹理尺寸
// #define DDGI_DEPTH_PROBE_SIZE 16                    //DDGI使用的深度贴图，单个probe的纹理尺寸

// #define HALF_SIZE_SSSR false                        //SSSR是否使用半精度

// #define VOLUMETRIC_FOG_SIZE_X 320                   //体积雾使用的屏幕空间texture3d的分辨率
// #define VOLUMETRIC_FOG_SIZE_Y 180
// #define VOLUMETRIC_FOG_SIZE_Z 128

// #define MAX_LIGHTS_PER_CLUSTER 8                    //每个cluster最多支持存储的光源数目
// #define TILE_BLOCK_PIX_SIZE 64                      //cluster light裁剪时使用的tile尺寸
// #define TILE_DEPTH_SIZE 128                         //cluster light裁剪时Z轴的划分数量    
// #define LIGHT_CLUSTER_NUM   (WINDOW_WIDTH / TILE_BLOCK_PIX_SIZE) * (WINDOW_HEIGHT / TILE_BLOCK_PIX_SIZE) * TILE_DEPTH_SIZE

// #define ENABLE_RAY_TRACING false					//是否启用光追



// #define VALIDATION_ENABLE true						//是否开启验证层

// #define MAX_BONE_INFLUENCE 4						//单顶点最大的骨骼影响数
// #define MAX_BONE_SIZE 200							//动画的最大骨骼数

// #define MAX_INSTANCE_COUNT 10240					//光追支持的最大实例数目



// #define MAX_ANISOTROPY 2							//最大各向异性，对性能影响还不小

// #define DIFFUSE_IBR_SIZE 32							//漫反射IBL尺寸
// #define SPECULAR_IBR_SIZE 128						//镜面反射IBL尺寸
// #define BRDF_LUT_SIZE 450							//BRDF_LUT尺寸



// #define CLUSTER_TRIANGLE_SIZE 128                   //每个cluster内的三角形数目，固定尺寸
// #define CLUSTER_GROUP_SIZE 32                       //每个cluster ghroup内的最大cluster数目
// #define MAX_GLOBAL_CLUSTER_SIZE 10240               //全局最大支持的cluster数目
// #define MAX_GLOBAL_CLUSTER_GROUP_SIZE 10240         //全局最大支持的cluster group数目
