

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

struct Object {

    mat4 prevModel;
    mat4 model; 

    uint animationID;           //动画buffer索引
    uint materialID;            //材质buffer索引
    uint positionID;
    uint normalID;
    uint tangentID;
    uint texCoordID;
    uint colorID;
    uint boneIndexID;              
    uint boneWeightID;
    uint indexID;             

    BoundingSphere sphere;  
    BoundingBox box;

    vec4 debugData;
};

struct Material {
  
    float roughness;
    float metallic;
    float alphaClip;

    vec4 baseColor;
    vec4 emission;

    int textureDiffuse;
    int textureNormal;
    int textureArm;        //AO/Roughness/Metallic
    int textureSpecular;

    int ints[8];       
    float floats[8];   

    vec4 colors[8];

    uvec4 textureSlots2D[2]; 
    uvec4 textureSlotsCube;
    uvec4 textureSlots3D;  
};

layout(set = 0, binding = 0) buffer global_setting {
    
    int skyboxMaterialID;
    int clusterInspectMode;    
    float totalTickTime;
    int totalTicks;

} GLOBAL_SETTING;

layout(set = 0, binding = 1) buffer camera {

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

layout(set = 0, binding = 2) buffer objects {

    Object slot[MAX_PER_FRAME_RESOURCE_SIZE];

} OBJECTS;

layout(set = 0, binding = 3) buffer materials { 

    Material slot[MAX_PER_FRAME_RESOURCE_SIZE];

} MATERIALS;

layout(set = 0, binding = 4) buffer positions { 

    float position[];

} POSITIONS[MAX_BINDLESS_RESOURCE_SIZE];

layout(set = 0, binding = 5) buffer normals { 

    float normal[];

} NORMALS[MAX_BINDLESS_RESOURCE_SIZE];


layout(set = 0, binding = 6) buffer tangents { 

    float tangent[];

} TANGENTS[MAX_BINDLESS_RESOURCE_SIZE];


layout(set = 0, binding = 7) buffer texCoords { 

    float texCoord[];

} TEXCOORDS[MAX_BINDLESS_RESOURCE_SIZE];


layout(set = 0, binding = 8) buffer colors { 

    float color[];

} COLORS[MAX_BINDLESS_RESOURCE_SIZE];


layout(set = 0, binding = 9) buffer boneIndexs { 

    int boneIndex[];

} BONEINDEXS[MAX_BINDLESS_RESOURCE_SIZE];

layout(set = 0, binding = 10) buffer boneWeights { 

    float boneWeight[];

} BONEWEIGHTS[MAX_BINDLESS_RESOURCE_SIZE];

layout(set = 0, binding = 11) buffer animations { 

    mat4 matrix[];

} ANIMATIONS[MAX_BINDLESS_RESOURCE_SIZE];

layout(set = 0, binding = 12) buffer indices { 

    uint index[];

} INDICES[MAX_BINDLESS_RESOURCE_SIZE];

layout(set = 0, binding = 13) uniform sampler1D TEXTURES_1D[];
layout(set = 0, binding = 14) uniform sampler1D TEXTURES_1D_ARRAY[];
layout(set = 0, binding = 15) uniform sampler2D TEXTURES_2D[];
layout(set = 0, binding = 16) uniform sampler2D TEXTURES_2D_ARRAY[];
layout(set = 0, binding = 17) uniform samplerCube TEXTURES_CUBE[];
layout(set = 0, binding = 18) uniform sampler3D TEXTURES_3D[];
