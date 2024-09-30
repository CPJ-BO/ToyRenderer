#ifndef LIGHT_GLSL
#define LIGHT_GLSL

// #define SHADOW_QUALITY_LOW    // 使用低质量阴影时定义

// cluster lighting 辅助函数////////////////////////////////////////////////////////////////////////////

ivec3 FetchLightClusterGrid(vec3 ndc)
{
    vec2 screenPixPos   = FetchScreenPixPos(ndc);

    float depth         = ndc.z;   
    float linearDepth   = LinearEyeDepth(depth, CAMERA.near, CAMERA.far);

    uint clusterZ       = uint(LIGHT_CLUSTER_DEPTH * log(linearDepth / CAMERA.near) / log(CAMERA.far / CAMERA.near));  //对数划分
    //uint clusterZ     = uint( (linearDepth - CAMERA.near) / ((CAMERA.far - CAMERA.near) / LIGHT_CLUSTER_DEPTH));   //均匀划分
    ivec3 clusterGrid   = ivec3(uint(screenPixPos.x / LIGHT_CLUSTER_GRID_SIZE), uint(screenPixPos.y / LIGHT_CLUSTER_GRID_SIZE), clusterZ);

    return clusterGrid;
}

ivec3 FetchLightClusterGrid(vec2 coord)
{
    float depth         = FetchDepth(coord);   
    vec3 ndc            = vec3(ScreenToNDC(coord), depth);

    return FetchLightClusterGrid(ndc);
}

uvec2 FetchLightClusterIndex(ivec3 clusterGrid)
{
    return imageLoad(LIGHT_CLUSTER_GRID, clusterGrid).xy;
}

// 阴影和衰减计算辅助函数/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float TextureProj(vec4 ndcPos, vec2 offset, texture2D tex)
{
	float shadow = 1.0;
	if ( ndcPos.z > -1.0 && ndcPos.z < 1.0 && ndcPos.x > -0.99 && ndcPos.x < 0.99 && ndcPos.y > -0.99 && ndcPos.y < 0.99) 
	{
		float dist = texture(sampler2D(tex, SAMPLER[0]), (ndcPos.xy + vec2(1.0)) * 0.5 + offset).r;
		if ( ndcPos.z > 0.0 && dist + 0.0002 < ndcPos.z ) 	//比较的时候还得加个小bias质量更好
		{
			shadow = 0;
		}
	}
	return shadow;
}

// PCF过滤 NDC空间坐标，纹理尺寸，纹理，层级
float FilterPCF(vec4 ndcPos, ivec2 texSize, texture2D tex)
{
	float scale = 1.0;
	float dx = scale * 1.0 / float(texSize.x);
	float dy = scale * 1.0 / float(texSize.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;	//非常影响性能，就滤波9个点吧
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += TextureProj(ndcPos, vec2(dx*x, dy*y), tex);
			count++;
		}
	}
	// return shadowFactor / count;
	return smoothstep(0, 1, shadowFactor / count);
}

float EVSM(vec4 moments, float depth, float c1, float c2)
{
	float x = exp(c1 * depth);
	float variance_x = max(moments.y - pow(moments.x, 2), 0.0001);
	float d_x = x - moments.x;  

	float p_x = variance_x / (variance_x + pow(d_x, 2)); 

	float y = exp(-c2 * depth);
	float variance_y = max(moments.w - pow(moments.z, 2), 0.0001);
	float d_y = y - moments.z;

	float p_y = variance_y / (variance_y + pow(d_y, 2));

	return min(p_x, p_y);
}

// 阴影和衰减计算////////////////////////////////////////////////////////////////////////////

int CascadeLevel(vec4 worldPos)
{
	vec3 viewPos = WorldToView(worldPos).xyz;

    int cascadeIndex = 0;
	for(int i = 0; i <= 3; i++) {
		if(-viewPos.z > LIGHTS.directionalLights[i].depth.x) {	
			cascadeIndex = i + 1;
		}
	}

	return cascadeIndex;
}

// 获取平行光源阴影值
float DirectionalShadow(vec4 worldPos)
{
	int cascadeIndex = CascadeLevel(worldPos);

    DirectionalLight dirLight = LIGHTS.directionalLights[cascadeIndex];

    vec3 dirLightVec = -normalize(dirLight.dir);    

    vec4 ndcPos = dirLight.proj * dirLight.view * worldPos;	
    ndcPos /= ndcPos.w;

#ifdef SHADOW_QUALITY_LOW

    float dirShadow = 1.0f;	
	float dist = texture(sampler2D(DIRECTIONAL_SHADOW[cascadeIndex], SAMPLER[0]), (ndcPos.xy + vec2(1.0)) * 0.5).r;
	if ( ndcPos.z > 0.0 && dist + 0.0002 < ndcPos.z ) 	//比较的时候还得加个小bias质量更好
	{
		dirShadow = 0.0f;
	}
	return dirShadow;

#else

	ivec2 texSize = textureSize(DIRECTIONAL_SHADOW[cascadeIndex], 0).xy;

    float dirShadow = FilterPCF(ndcPos, texSize, DIRECTIONAL_SHADOW[cascadeIndex]);	//计算平行光源阴影取值，[0,1]
	dirShadow = smoothstep(0.0f, 1.0f, min(1.0, dirShadow / 0.9));	                            //重映射

	return dirShadow;

#endif
}

// 获取点光源阴影值
float PointShadow(in PointLight pointLight, vec4 worldPos)
{
	if(pointLight.shadowID >= MAX_POINT_SHADOW_COUNT) return 1.0f;	//无效阴影索引

	vec3 pointLightVec = normalize(pointLight.pos - worldPos.xyz);
	float pointDistance = length(worldPos.xyz - pointLight.pos);

#ifdef SHADOW_QUALITY_LOW	// 直接用深度

	float pointShadow =  pointDistance / pointLight.far - texture(samplerCube(POINT_SHADOW[pointLight.shadowID], SAMPLER[0]), -pointLightVec).r;
	pointShadow = pointShadow > pointLight.bias ? 0.0f : 1.0f;	//给阴影一点颜色？

#else	// 使用EVSM

	vec4 moments = texture(samplerCube(POINT_SHADOW[pointLight.shadowID], SAMPLER[0]), -pointLightVec);

	float depth = pointDistance / pointLight.far;

	float pointShadow = EVSM(moments, depth, pointLight.c1, pointLight.c2);		//计算点光源阴影取值，[0,1]
	pointShadow = smoothstep(0.0f, 1.0f, min(1.0, pointShadow / 0.8));	        //重映射，去掉大于0.8的部分

#endif

    return pointShadow;
}

float PointLightFalloff(float dist, float radius)
{
	return pow(clamp(1.0f - pow(dist / radius, 4), 0.0, 1.0), 2) / (dist * dist + 1.0f);
}

#endif