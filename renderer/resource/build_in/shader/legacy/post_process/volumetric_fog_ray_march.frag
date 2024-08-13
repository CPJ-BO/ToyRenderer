#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_frag.glsl"

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 color;

#define STEP 0.01

#define attenuation_factor 0.1     // 衰减系数 = 外散射系数 + 吸收率

#define PI 3.1415926

// Beer–Lambert law 透光率与传输距离成指数关系
float transmittance(vec3 worldPos, vec3 cameraPos)
{
    return exp(-distance(worldPos.xyz, cameraPos.xyz) * attenuation_factor);
}

// 相位函数，光被散射到各方向的分布函数
// 各向异性近似
float henyeyGreensteinPhase(float G, float CosTheta)
{
    // Reference implementation (i.e. not schlick approximation). 
    // See http://www.pbr-book.org/3ed-2018/Volume_Scattering/Phase_Functions.html
    float Numer = 1.0f - G * G;
    float Denom = max(0.001,1.0f + G * G - 2.0f * G * CosTheta);
    return Numer / (4.0f * PI * Denom * sqrt(Denom));
}

// 各向同性
float isotropicScattering()
{
    return 1.0f / (4 * PI);
}



void main() 
{
    //vec4 worldPos     = depthToWorld(inUV);
    //float depth       = fetchDepth(inUV);
    //float linearDepth = LinearEyeDepth(depth, CAMERA.near_far.x, CAMERA.near_far.y);

    //vec4 worldNearPos   = screenToWorld(inUV, 0);    //最近点
    vec4 worldNearPos   = CAMERA.pos;                //最近点
    vec4 worldFarPos    = depthToWorld(inUV);        //最远点
    float worldDist     = length(worldFarPos.xyz - worldNearPos.xyz);
    vec3 stepVec        = normalize((worldNearPos - worldFarPos).xyz);

    vec4 viewNearPos    = worldToView(worldNearPos);    
    vec4 viewFarPos     = worldToView(worldFarPos);      

    vec4 ndcNearPos     = viewToNdc(viewNearPos);
    vec4 ndcFarPos      = viewToNdc(viewFarPos);

    vec3 accumulateColor = vec3(0.0f);




    vec4 currentWorldPos = worldFarPos;  


    vec2 noise          = fetchNoise2D(ivec2(fetchScreenPixPos(inUV)));
    float totalTickTime = GLOBAL_SETTING.totalTickTime;
    float random = smoothstep(0.0f, 1.0f, random(vec2(noise.x + totalTickTime)));    //作为起始的抖动

    float minMarchCount = 50;
    float dis = min(1.0f, worldDist / minMarchCount);   //步进间隔, 至少保证minMarchCount次march


    for(int round = 0; round < worldDist / dis; round++)
    {
        currentWorldPos.xyz = worldFarPos.xyz + stepVec * dis * (round + random);   // 从近至远
        //currentWorldPos.xyz += stepVec * dis;  

        // 点光源
        {
            ivec3 clusterID     = fetchLightClusterID(worldToNdc(currentWorldPos).xyz);
            uvec2 clusterIndex  = fetchLightClusterIndex(clusterID);

            for(uint i = clusterIndex.x; i < clusterIndex.x + clusterIndex.y; i++)
            {
                PointLight pointLight = LIGHT.pointLights[LIGHT_INDEX.slot[i].index];

                float pointDistance = length(currentWorldPos.xyz - pointLight.pos.xyz);
                float pShadow = pointShadow(pointLight, currentWorldPos.xyz, true);  

                float attenuation   = pointLightFalloff(pointDistance, pointLight.near_far_bias[1]);

                vec3 radiance     = pShadow * pointLight.color_intencity.a * pointLight.color_intencity.rgb * attenuation; 

                accumulateColor += radiance * isotropicScattering() * transmittance(currentWorldPos.xyz, worldNearPos.xyz);
                //accumulateColor += pointShadow * attenuation;
                //accumulateColor += pointShadow;
            }
        }

        // 平行光源
        if(false)
        {
            DirectionalLight dirLight = LIGHT.directionalLights[0];

            float dirShadow = directionalShadow(currentWorldPos.xyz, true);           

            vec3 radiance = dirShadow * 
                                dirLight.color_intencity.a * 
                                dirLight.color_intencity.rgb;

            accumulateColor += radiance * isotropicScattering() * transmittance(currentWorldPos.xyz, worldNearPos.xyz);
        }
    }





    color = vec4(accumulateColor, 1 - transmittance(worldFarPos.xyz, worldNearPos.xyz));
    //color.xyz /= 5;
    color.w = 1.0f;
}



// float4 ScatterStep(float3 accumulatedLight, float accumulatedTransmittance, float3 sliceLight, float sliceDensity)
// {
//     sliceDensity = max(sliceDensity, 0.000001);
//     float  sliceTransmittance = exp(-sliceDensity / _GridSizeZ);

//     // Seb Hillaire's improved transmission by calculating an integral over slice depth instead of
//     // constant per slice value. Light still constant per slice, but that's acceptable. See slide 28 of
//     // Physically-based & Unified Volumetric Rendering in Frostbite
//     // http://www.frostbite.com/2015/08/physically-based-unified-volumetric-rendering-in-frostbite/
//     float3 sliceLightIntegral = sliceLight * (1.0 - sliceTransmittance) / sliceDensity;

//     accumulatedLight += sliceLightIntegral * accumulatedTransmittance;
//     accumulatedTransmittance *= sliceTransmittance;

//     return float4(accumulatedLight, accumulatedTransmittance);
// }