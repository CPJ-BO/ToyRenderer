#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_frag.glsl"

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 color;

layout(push_constant) uniform sSSRSetting {
    int max_mip;
    int start_mip;
    int end_mip;
	float thickness;
    int max_loop;
    float scale;

    float blend;
    float cross_epsilon;
} SSSRSetting;


bool validUV(vec2 uv, vec2 border)
{
    return  uv.x <= 1.0f - border.x && uv.x >= 0.0f + border.x && 
            uv.y <= 1.0f - border.y && uv.y >= 0.0f + border.y;
}

vec2 mipSize(int mipLevel)
{
    return textureSize(DEPTH_PYRAMID_SAMPLER_MIN, mipLevel).xy;
}

vec3 intersectDepthPlane(vec3 o, vec3 d, float t)
{
	return o + d * t;
}

vec2 getCell(vec2 ray, vec2 cellCount)
{
	return floor(ray * cellCount);
}

vec2 getCellCount(int level)
{
	return textureSize(DEPTH_PYRAMID_SAMPLER_MIN, level).xy;
}

float sampleDepth(sampler2D depth, int mipLevel, vec2 uv)
{
    vec2 cellSize = mipSize(mipLevel);
    vec2 deltaUV = 1.0 / cellSize;
    float minDepth[5];

    float color00 = texelFetch( depth, ivec2(floor(uv * cellSize + vec2(-1, -1))), mipLevel ).r;
	float color01 = texelFetch( depth, ivec2(floor(uv * cellSize + vec2(-1, 0))), mipLevel ).r;
	float color02 = texelFetch( depth, ivec2(floor(uv * cellSize + vec2(-1, 1))), mipLevel ).r;
	float color10 = texelFetch( depth, ivec2(floor(uv * cellSize + vec2(0, -1))), mipLevel ).r;
    float color11 = texelFetch( depth, ivec2(floor(uv * cellSize + vec2(0, 0))), mipLevel ).r;
	float color12 = texelFetch( depth, ivec2(floor(uv * cellSize + vec2(0, 1))), mipLevel ).r;
	float color20 = texelFetch( depth, ivec2(floor(uv * cellSize + vec2(1, -1))), mipLevel ).r;
	float color21 = texelFetch( depth, ivec2(floor(uv * cellSize + vec2(1, 0))), mipLevel ).r;
    float color22 = texelFetch( depth, ivec2(floor(uv * cellSize + vec2(1, 1))), mipLevel ).r;
    return min(min(min(min(min(min(min(min(color00, color01), color02), color10), color11), color12), color20), color21), color22);
            

    // minDepth[0] = texture(depth, uv + vec2(-1, -1) * deltaUV, mipLevel).r;
    // minDepth[1] = texture(depth, uv + vec2(-1, 1) * deltaUV, mipLevel).r;
    // minDepth[2] = texture(depth, uv + vec2(1, -1) * deltaUV, mipLevel).r;
    // minDepth[3] = texture(depth, uv + vec2(1, 1) * deltaUV, mipLevel).r;

    // return min(min(min(minDepth[0], minDepth[1]), minDepth[2]), minDepth[3]);
}


float getDepth(vec2 ray, int level)
{
    // if(level > 0)   return (texture(DEPTH_PYRAMID_SAMPLER_MIN,  ray.xy, level).r);    
    // else            return (texture(DEPTH_SAMPLER,              ray.xy, level).r);

    if(level > 0)   return sampleDepth(DEPTH_PYRAMID_SAMPLER_MIN,   level, ray.xy);
    else            return sampleDepth(DEPTH_SAMPLER,               level, ray.xy);

    // vec2 cellSize = mipSize(level);
    // if(level > 0)   return texelFetch( DEPTH_PYRAMID_SAMPLER_MIN,   ivec2(floor(ray * cellSize)), level ).r;
    // else            return texelFetch( DEPTH_SAMPLER,               ivec2(floor(ray * cellSize)), level ).r;
}


vec3 intersectCellBoundary(vec3 o, vec3 d, vec2 cellIndex, vec2 cellCount, vec2 crossStep, vec2 crossOffset)
{
	vec2 index  = cellIndex + crossStep;
	index       /= cellCount;
	index       += crossOffset;

	vec2 delta  = index - o.xy;
	delta       /= d.xy;

	float t = min(delta.x, delta.y);

	return intersectDepthPlane(o, d, t);
}

bool crossedCellBoundary(vec2 cellIdxOne, vec2 cellIdxTwo)
{
	return cellIdxOne.x != cellIdxTwo.x || cellIdxOne.y != cellIdxTwo.y;
}

#define HIZ_CROSS_EPSILON 0.01

vec3 hiZTrace(vec3 p, vec3 v, out vec4 debug)
{
	// get the cell cross direction and a small offset to enter the next cell when doing cell crossing
	vec2 crossStep = vec2(v.x >= 0.0f ? 1.0f : -1.0f, v.y >= 0.0f ? 1.0f : -1.0f);
	vec2 crossOffset = vec2(crossStep.xy * SSSRSetting.cross_epsilon);
	//crossStep.xy = Saturate(crossStep.xy);

	// set current ray to original screen coordinate and depth
	vec3 ray = p.xyz;

	// scale vector such that z is 1.0f (maximum depth)
	vec3 d = v.xyz / v.z;

	// set starting point to the point where z equals 0.0f (minimum depth)
	vec3 o = intersectDepthPlane(p, d, -p.z);

	// cross to next cell to avoid immediate self-intersection
	vec2 rayCell = getCell(ray.xy, getCellCount(0));
	ray = intersectCellBoundary(o, d, rayCell.xy, getCellCount(0), crossStep.xy, crossOffset.xy);


 
    float currentZ = 0.0f;
    int level = SSSRSetting.start_mip;
    int iterations = 0;
    ivec4 levelCnt      = ivec4(0);
	while(  level >= SSSRSetting.end_mip && 
            iterations < SSSRSetting.max_loop)
	{

        if(level < 4) levelCnt[level]++;

		// get the minimum depth plane in which the current ray resides
		float newZ = getDepth(ray.xy, level);
		
		// get the cell number of the current ray
		vec2 cellCount = getCellCount(level);
		vec2 oldCellIdx = getCell(ray.xy, cellCount);

		// intersect only if ray depth is below the minimum depth plane
		//vec3 tmpRay = intersectDepthPlane(o, d, max(ray.z, newZ));          //这里的z不是通过插值得到的，而是直接存上一轮的
        //vec3 tmpRay = (ray.z < newZ) ? intersectDepthPlane(o, d, newZ) : ray;         


        if(ray.z < newZ)
        {
            vec3 tmpRay = intersectDepthPlane(o, d, newZ);   

            vec2 newCellIdx = getCell(tmpRay.xy, cellCount);
            if(crossedCellBoundary(oldCellIdx, newCellIdx))
            {
                tmpRay = intersectCellBoundary(o, d, oldCellIdx, cellCount.xy, crossStep.xy, crossOffset.xy); 
                level = min(SSSRSetting.max_mip, level + 2);
            }
            ray.xyz = tmpRay.xyz;
            currentZ = newZ;
        }

		level--;
		iterations++;
	} 

    debug.x = iterations;
    //debug.yzw = levelCnt.xyz;

    float sampleZ   = getDepth(ray.xy, 0);

    // debug.y = currentZ;
    // debug.z = sampleZ;
    // debug.w = ray.z;

    debug.y = LinearEyeDepth(currentZ, CAMERA.near_far.x, CAMERA.near_far.y);
    debug.z = LinearEyeDepth(sampleZ, CAMERA.near_far.x, CAMERA.near_far.y);
    debug.w = abs(debug.y - debug.z);

	return ray;






}

void main()
{
    vec4 screenColor    = texture(COLOR_0_SAMPLER, inUV);
    vec4 reflectColor   = vec4(0.0f);

    vec3 worldNormal    = fetchNormal(inUV);    
    vec4 worldPos       = vec4(depthToWorld(inUV).xyz, 1.0f);

    vec3 eyeVec        = normalize(CAMERA.pos.xyz - worldPos.xyz);		
    vec3 reflectVev    = normalize(reflect(-eyeVec, worldNormal)) * 1;        

    vec4 targetWorldPos = vec4(worldPos.xyz + reflectVev, 1.0f);    

    if(length(worldNormal) < 0.01)    //没有有效的法线信息
    {
        color = screenColor;
        return;
    }



    // 步进起始点             
    vec4 baseClipPos    = worldToCLip(worldPos);   
    float baseW         = 1.0 / baseClipPos.w;                                    
    float baseD         = baseClipPos.z * baseW;
    vec2 baseUV         = ndcToScreen(baseClipPos.xy * baseW);                 

    // 步进方向
    vec4 targetClipPos  = worldToCLip(targetWorldPos);
    float targetW       = 1.0 / targetClipPos.w;                    
    float targetD       = targetClipPos.z * targetW;       
    vec2 targetUV       = ndcToScreen(targetClipPos.xy * targetW);  

    // 步进单位
    // float deltaScale    = 1 / length(targetUV - baseUV);            //单位长度
    // vec2 deltaUV        = (targetUV - baseUV)   * deltaScale;            
    // float deltaW        = (targetW - baseW)     * deltaScale;             
    // float deltaDr       = (targetDr - baseDr)   * deltaScale;


    vec3 positionSS = vec3(baseUV, baseD);                      //Texture Space
    vec3 reflectSS  = vec3(targetUV, targetD) - positionSS; 

    // Hiz加速////////////////////////////////////////////

    vec4 debug;
	vec3 raySS = hiZTrace(positionSS, reflectSS, debug);




    if( validUV(raySS.xy, vec2(0.01f)) && 
        raySS.z < 0.999 &&
        debug.w < SSSRSetting.thickness)   reflectColor = texture(COLOR_0_SAMPLER, raySS.xy);    

    //reflectColor = vec4(raySS, 1.0f);
    //reflectColor = debug;

    //color               = screenColor;   
    //color               = Lerp(screenColor, reflectColor, SSSRSetting.blend); 
    //color               = screenColor + reflectColor * SSSRSetting.blend ;
    color                   = reflectColor;
}

