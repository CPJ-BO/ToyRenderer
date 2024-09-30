#ifndef COMMON_LIGHT_GLSL
#define COMMON_LIGHT_GLSL


//阴影和衰减计算辅助函数/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float TextureProj(vec4 ndcPos, vec2 offset, sampler2DArray tex, int layer)
{
	float shadow = 1.0;
	if ( ndcPos.z > -1.0 && ndcPos.z < 1.0 && ndcPos.x > -0.99 && ndcPos.x < 0.99 && ndcPos.y > -0.99 && ndcPos.y < 0.99) 
	{
		float dist = texture(tex, vec3((ndcPos.xy + vec2(1.0) + offset) * 0.5, layer)).r;
		if ( ndcPos.z > 0.0 && dist + 0.0002 < ndcPos.z ) 	//比较的时候还得加个小bias质量更好
		{
			shadow = 0;
		}
	}
	return shadow;
}

//PCF过滤 NDC空间坐标，纹理尺寸，纹理，层级
float Filter_PCF(vec4 ndcPos, ivec2 texSize, sampler2DArray tex, int layer)
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
			shadowFactor += TextureProj(ndcPos, vec2(dx*x, dy*y), tex, layer);
			count++;
		}
	}
	//return shadowFactor / count;
	return smoothstep(0, 1, shadowFactor / count);
}

float EVSM(vec4 moments, float depth, vec2 c1c2, float bias)
{
	float x = exp(c1c2.x * depth);
	float variance_x = max(moments.y - pow(moments.x, 2), 0.0001);
	float d_x = x - moments.x;  

	float p_x = variance_x / (variance_x + pow(d_x, 2)); 

	float y = exp(-c1c2.y * depth);
	float variance_y = max(moments.w - pow(moments.z, 2), 0.0001);
	float d_y = y - moments.z;

	float p_y = variance_y / (variance_y + pow(d_y, 2));

	return min(p_x, p_y);
}

//阴影和衰减计算////////////////////////////////////////////////////////////////////////////

//获取平行光源阴影值
float directionalShadow(vec3 worldPos)
{
	vec3 viewPos = worldToView(vec4(worldPos, 1.0f)).xyz;

    int cascadeIndex = 0;
	for(int i = 0; i <= 3; i++) {
		//if((CAMERA.view * inPos).z > 0) {	
		if(-viewPos.z > LIGHT.directionalLights[i].depth.x) {	
			cascadeIndex = i + 1;
		}
	}

    DirectionalLight dirLight = LIGHT.directionalLights[cascadeIndex];

    vec3 dirLightVec = -normalize(dirLight.dir);    

    vec4 ndcPos = dirLight.proj * dirLight.view * vec4(worldPos, 1.0);	
    ndcPos /= ndcPos.w;

	ivec2 texSize = textureSize(DIRECTIONAL_SHADOW_SAMPLER, 0).xy;

    float dirShadow = Filter_PCF(ndcPos, texSize, DIRECTIONAL_SHADOW_SAMPLER, cascadeIndex);	//计算平行光源阴影取值，[0,1]
	dirShadow = smoothstep(0.0f, 1.0f, min(1.0, dirShadow / 0.9));	                            //重映射

	return dirShadow;
}

float directionalShadow_low(vec3 worldPos)		//不使用PCF直接采样
{
	vec3 viewPos = worldToView(vec4(worldPos, 1.0f)).xyz;

    int cascadeIndex = 0;
	for(int i = 0; i <= 3; i++) {
		//if((CAMERA.view * inPos).z > 0) {	
		if(-viewPos.z > LIGHT.directionalLights[i].depth.x) {	
			cascadeIndex = i + 1;
		}
	}

    DirectionalLight dirLight = LIGHT.directionalLights[cascadeIndex];

    vec3 dirLightVec = -normalize(dirLight.dir);    

    vec4 ndcPos = dirLight.proj * dirLight.view * vec4(worldPos, 1.0);	
    ndcPos /= ndcPos.w;

    float dirShadow = 1.0f;	

	float dist = texture(DIRECTIONAL_SHADOW_SAMPLER, vec3((ndcPos.xy + vec2(1.0)) * 0.5, cascadeIndex)).r;
	if ( ndcPos.z > 0.0 && dist + 0.0002 < ndcPos.z ) 	//比较的时候还得加个小bias质量更好
	{
		dirShadow = 0.0f;
	}

	return dirShadow;
}

float directionalShadow(vec3 worldPos, bool lowShadowQuality)
{
	if(lowShadowQuality)	return	directionalShadow_low(worldPos);
	else					return	directionalShadow(worldPos);
}

//获取点光源阴影值
float pointShadow(PointLight pointLight, vec3 worldPos)
{
	if(pointLight.shadowID >= POINT_SHADOW_MAX_COUNT) return 1.0f;	//无效阴影索引

	vec3 pointLightVec = normalize(pointLight.pos - worldPos);
	float pointDistance = length(worldPos - pointLight.pos);

	// 直接用深度
	// float pointShadow =  pointDistance / pointLight.near_far_bias.y - texture(POINT_SHADOW_SAMPLERS[pointLight.shadowID], -pointLightVec).r;
	// pointShadow = pointShadow > pointLight.near_far_bias.z ? 0.0f : 1.0f;	//给阴影一点颜色？
		
	// 使用EVSM
	vec4 moments = texture(POINT_SHADOW_SAMPLERS[pointLight.shadowID], -pointLightVec);

	float depth = pointDistance / pointLight.near_far_bias.y;

	float pointShadow = EVSM(moments, depth, pointLight.evsm_c1c2.xy, pointLight.near_far_bias.z);	//计算点光源阴影取值，[0,1]
	pointShadow = smoothstep(0.0f, 1.0f, min(1.0, pointShadow / 0.8));	                            //重映射，去掉大于0.8的部分

    return pointShadow;
}

float pointShadow_low(PointLight pointLight, vec3 worldPos)	// 直接用深度计算的版本
{
	if(pointLight.shadowID >= POINT_SHADOW_MAX_COUNT) return 1.0f;	//无效阴影索引

	vec3 pointLightVec = normalize(pointLight.pos - worldPos);
	vec3 pointDistance = abs(worldPos - pointLight.pos);	

	float worldDistance = max(max(pointDistance.x, pointDistance.y), pointDistance.z);	//该点到光源的世界空间“深度”（平行xyz轴的距离）

	float shadowDistance = LinearEyeDepth(texture(POINT_SHADOW_DEPTH_SAMPLERS[pointLight.shadowID], -pointLightVec).r, pointLight.near_far_bias.x, pointLight.near_far_bias.y);

	float pointShadow = worldDistance - shadowDistance;
	pointShadow = pointShadow > pointLight.near_far_bias.z ? 0.0f : 1.0f;	//给阴影一点颜色？

    return pointShadow;
}

float pointShadow(PointLight pointLight, vec3 worldPos, bool lowShadowQuality)
{
	if(lowShadowQuality)	return	pointShadow_low(pointLight, worldPos);
	else					return	pointShadow(pointLight, worldPos);
}

float pointLightFalloff(float dist, float radius)
{
	return pow(clamp(1.0f - pow(dist / radius, 4), 0.0, 1.0), 2) / (dist * dist + 1.0f);
}


#endif