#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_frag.glsl"

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 color;

layout(push_constant) uniform sSSRSetting {
	float step;
    float blend;
    float fall_off_distance;
    float max_mip;
} SSSRSetting;









bool validUV(vec2 uv)
{
    return  uv.x <= 1.0f && uv.x >= 0.0f && 
            uv.y <= 1.0f && uv.y >= 0.0f;
}

vec2 mipSize(int mipLevel)
{
    return textureSize(DEPTH_PYRAMID_SAMPLER_MIN, mipLevel).xy;
}

// bool crossedCellBoundary(vec2 uv0, vec2 uv1, int mipLevel)
// {
//     vec2 mipSize = mipSize(mipLevel);
// 	return round(uv0 * mipSize) != round(uv1 * mipSize);
// }

// vec4 clampToCell(vec2 uv, vec2 deltaUV, int mipLevel, out vec2 finalUV, out vec2 cellCenterUV, out float scale)
// {
//     vec2 texSize    = mipSize(mipLevel);   //以下全部计算使用像素坐标，取值范围[WINDOW_WIDTH, WINDOW_HEIGHT]，0为像素中心，0.5为像素边缘

//     vec3 lineBegin  = vec3(texSize * (uv - normalize(deltaUV) * 0.1), 0.0f);
//     vec3 lineEnd    = vec3(texSize * (uv + normalize(deltaUV) * 1.0), 0.0f);
   
//     vec2 cellCenter = round(texSize * uv);      //从起点开始位移
//     cellCenterUV    = cellCenter / texSize;                                    

//     vec3 cellMin    = vec3(cellCenter - 0.5f, -1.0f); 
//     vec3 cellMax    = vec3(cellCenter + 0.5f, 1.0f);
           
//     vec2 intersect  = LineIntersectBox(lineBegin, lineEnd, cellMin, cellMax);    //用的三维的相交函数，保证第三维能交上就行，交到边界较远点

//     finalUV         = Lerp(lineBegin.xy, lineEnd.xy, intersect.y);
//     finalUV         += normalize(deltaUV) * 0.01;                                //给最终的UV多一点点位移，偏移出边界
//     finalUV         /= texSize;

//     scale           = length(finalUV - uv) / length(deltaUV);


//     //return vec4(cellMin.xy, cellMax.xy);
//     return vec4(cellCenter - lineBegin.xy, Lerp(lineBegin.xy, lineEnd.xy, intersect.y));
// }


float getMarchSize(vec2 start, vec2 end, vec2 sampleUV)
{
    vec2 dir = abs(end - start);
    return length( vec2( min(dir.x, sampleUV.x), min(dir.y, sampleUV.y) ) );    //位移长度，一个格子对角线的长度
}



void main()
{
    vec4 screenColor    = texture(COLOR_0_SAMPLER, inUV);
    vec4 reflectColor   = vec4(0.0f);

    vec3 worldNormal    = fetchNormal(inUV);    
    vec4 worldPos       = vec4(depthToWorld(inUV).xyz, 1.0f);

    vec3 eyeVec        = normalize(CAMERA.pos.xyz - worldPos.xyz);		
    vec3 reflectVev    = normalize(reflect(-eyeVec, worldNormal));        

    vec4 targetWorldPos = vec4(worldPos.xyz + reflectVev, 1.0f);    

    if(length(worldNormal) < 0.01)    //没有有效的法线信息
    {
        color = screenColor;
        return;
    }



    // 步进起始点             
    vec4 baseClipPos    = worldToCLip(worldPos);   
    float baseW         = 1.0 / baseClipPos.w;                                    
    float baseDr        = 1.0 / (baseClipPos.z * baseW);
    vec2 baseUV         = ndcToScreen(baseClipPos.xy * baseW);                 

    // 步进方向
    vec4 targetClipPos  = worldToCLip(targetWorldPos);
    float targetW       = 1.0 / targetClipPos.w;                    //ndc空间的w坐标倒数，线性
    float targetDr      = 1.0 / (targetClipPos.z * targetW);        //ndc空间的深度倒数，线性
    vec2 targetUV       = ndcToScreen(targetClipPos.xy * targetW);  //uv坐标，线性

    // 步进单位
    vec2 deltaUV        = targetUV - baseUV;            
    float deltaW        = targetW - baseW;             
    float deltaDr       = targetDr - baseDr;
    float deltaDist     = length(deltaUV);

    //vec2 texSize        = 1.0 / mipSize(0);
    //float resize        = length(texSize) / length(deltaUV);

    // deltaUV             *= SSSRSetting.step;
    // deltaW              *= SSSRSetting.step;
    // deltaDr             *= SSSRSetting.step;
    // deltaDist           *= SSSRSetting.step;
   
    // 测试点
    vec2 testUV         = baseUV;
    float testDr        = baseDr;
    float testDist      = 0.0f;

    float loop          = 0;
    bool hit            = false;
    

    // 暴力循环////////////////////////////////////////////
    // float stride        = 1.0;
    // while(  testUV.x <= 1.0f && testUV.x >= 0.0f && 
    //         testUV.y <= 1.0f && testUV.y >= 0.0f &&         //每轮步进的世界空间距离：SSSRSetting.step * stride
    //         loop < 1000 && stride > 0.005)
    // {
    //     loop++;

    //     vec2 tempUV     = testUV + deltaUV * stride;
    //     float tempDr    = testDr + deltaDr * stride;
    //     vec2 range      = vec2(tempDr, testDr);             //深度的倒数，越远越小

    //     float sampleDr = 1.0 / fetchDepth(tempUV);

    //     if(tempDr - 1.0 < 0.0001) break;                     //无穷远处
    //     if(sampleDr < range.x)                               //测试步进未被遮挡，移动测试点
    //     {
    //         testUV += deltaUV * stride;
    //         testDr += deltaDr * stride;
    //         testDist += deltaDist * stride;
    //     }
    //     else if(range.x < sampleDr && sampleDr < range.y)   //测试步进命中，缩短测试距离
    //     {
    //         stride /= 2;
    //         hit = true;
    //     }
    //     else stride /= 2;   
    // }

    // if(hit) reflectColor = texture(COLOR_0_SAMPLER, testUV);  


    // 每次march最多到对应mip层级的边缘，采样时采mip像素中心，否则扩大步长可能导致越过本来被遮挡的地方
    // Hiz加速////////////////////////////////////////////


    // vec2 tempUV;
    // vec2 cellCenter;
    // float scale;
    // clampToCell(testUV, deltaUV, 0, tempUV, cellCenter, scale);    //执行之前先march到边界一次

    // testUV += deltaUV * scale;
    // testDr += deltaDr * scale;
    // testDist += deltaDist * scale;



    int hizLevel = 0;
    while( hizLevel >= 0 && loop < 5000)  
    {
        loop++;

        vec2 outUV;
        vec2 cellCenterUV;
        float scale;
        clampToCell(testUV, deltaUV, hizLevel, outUV, cellCenterUV, scale);

        float tempDr    = testDr + deltaDr * scale;
        vec2 range      = vec2(tempDr, testDr);             //深度的倒数，越远越小

        float sampleDr = 0.0;
        if(hizLevel > 0)    sampleDr = 1.0 / (texture(DEPTH_PYRAMID_SAMPLER_MIN, cellCenterUV, hizLevel).r);
        else                sampleDr = 1.0 / fetchDepth(cellCenterUV);

        if(!validUV(outUV))    hizLevel--;                          //0. 超出UV范围，缩短测试距离
        else if(sampleDr < range.x)                                 //1. 测试步进未被遮挡，移动测试点
        {
            testUV += deltaUV * scale;
            testDr += deltaDr * scale;
            testDist += deltaDist * scale;

            if(SSSRSetting.max_mip > hizLevel && !hit)  hizLevel++; //如果之前已经命中，那一定不会超过之前的层级了
        }
        else if(range.x <= sampleDr && sampleDr < range.y)          //2. 测试步进命中，缩短测试距离
        {          
            hit = true;
            hizLevel--;                     
        }
        else hizLevel--;                                            //3. 比位移前都要大？？
    }


    float sampleDr  = 1.0 / fetchDepth(testUV);
    if(sampleDr - 1.0 < 0.0001)     reflectColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);        //蓝色 无限远处
    else if(hit)                    reflectColor = texture(COLOR_0_SAMPLER, testUV);  
    reflectColor = texture(COLOR_0_SAMPLER, testUV);  
    //else if(hit)                    reflectColor = vec4(testUV, mipSize(0));            //白色 命中
    //else if(stride <= 0.005)        reflectColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);        //红色 步长到头了没找到
    //else if(loop == 1000)                reflectColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);    //绿色 循环到头了没找到
    //reflectColor = vec4(loop, 0, 0, 0);     




    // vec2 tempUV;
    // vec2 cellCenterUV;
    // float scale;

    // float depths[5];
    // float scales[5];
    // vec2 cellCenters[5];

    // for(int i = 0; i < 5; i++)
    // {
    //     clampToCell(testUV, deltaUV, 0, tempUV, cellCenterUV, scale);
    //     depths[i] = fetchDepth(cellCenterUV);
    //     scales[i] = scale;
    //     cellCenters[i] = cellCenterUV;

    //     testUV += deltaUV * scale;
    // }

    // reflectColor = vec4(    depths[1] - depths[0], 
    //                         depths[2] - depths[1], 
    //                         depths[3] - depths[2], 
    //                         depths[4] - depths[3]);  

    // reflectColor = vec4(    scales[0], 
    //                         scales[1], 
    //                         scales[2], 
    //                         scales[3]);  

    // reflectColor = vec4(    length(cellCenters[1] - cellCenters[0]), 
    //                         length(cellCenters[2] - cellCenters[1]), 
    //                         length(cellCenters[3] - cellCenters[2]), 
    //                         length(cellCenters[4] - cellCenters[3]));  







    //color               = screenColor;   
    //color               = Lerp(screenColor, reflectColor, SSSRSetting.blend); 
    //color               = screenColor + reflectColor * SSSRSetting.blend ;
    color                   = reflectColor;
}

